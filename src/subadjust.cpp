
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>

#ifdef WIN32
#include <windows.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/fl_ask.H>
#include <FL/platform.H>

#include "Fl_Time_Input.H"
#include "app_info.h"
#include "edit_features.h"
#include "file_features.h"
#include "log.h"
#include "options.h"
#include "pref.h"
#include "subadjust_icon.h"
#include "subadjust_ui.h"
#include "subs.h"
#include "themes.h"
#include "utils.h"

Fl_SVG_Image svg(0, svg_data);

void about_msg(Fl_Widget *, void *v)
{
  options *opt = (options *)v;
  std::string about_msg = opt->Progname + " version " + app_info.version;

  if (!app_info.decoration.empty())
    about_msg += ' ' + app_info.decoration;
  about_msg += '\n' + app_info.copyright;

  if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R))
  {
    std::string traceability("");

    if (!app_info.commit.empty())
      traceability += app_info.commit;

    traceability += " built for " + app_info.platform;
    if (!app_info.created_at.empty())
      traceability += " on " + app_info.created_at;

    Fl::copy(traceability.c_str(), (int)traceability.size(), 1);
    about_msg += "\n[" + traceability + ']';
  }

  // svg.scale(18, 18); fl_message_icon_label(""); fl_message_icon()->image(svg);
  fl_message_position(main_window->x_root(), main_window->y_root() + 60, 0);
  //  fl_message("%s", about_msg.c_str());
  if (!fl_choice("%s", "More info ...", "Ok", 0L, about_msg.c_str()))
  {
    fl_message_position(main_window->x_root(), main_window->y_root() + 60, 0);
    Fl_Font actual_font = fl_message_font_;
    Fl_Fontsize actual_size = fl_message_size_;
    // #include <FL/names.h>
    //     logI("actual_font: ", fl_fontnames[actual_font], ", actual_size: ", actual_size);
    fl_message_font(FL_COURIER_BOLD, 10);
    fl_message("%s", opt->usage().c_str());
    fl_message_font(actual_font, actual_size);
  }
}

double get_reset_val(Fl_Float_Input *w, double reset_val)
{
  double d;
  if (w->value() != nullptr)
    d = std::stod(w->value());
  else
    d = reset_val;
  w->value(reset_val);
  return d;
}

void subadjust(Fl_Widget *, void *)
{
  int beg_ts = time_start->get_time_ms(), end_ts = time_end->get_time_ms();

  csub.parse(txt_buf.text());

  double offs_start = get_reset_val(offset_start, 0);
  double offs_stop = get_reset_val(offset_start, 0);
  double dur_k = get_reset_val(dur_coeff, 1);

  if (csub.adjust(beg_ts, end_ts, offs_start, offs_stop, dur_k))
  {
    time_start->set_time_ms(csub.vec()[0].appearance);
    time_end->set_time_ms(csub.vec().back().appearance);
    txt_buf.text(csub.c_str());
    file_content->redraw();
    set_file_state(true);
    if (csub.err_msg != "")
      fl_message("%s", csub.err_msg.c_str());
  }
  else
    fl_alert("%s", csub.err_msg.c_str());
}

void quit_cb(Fl_Widget *, void *)
{
  static bool already_done = false;

  if (!already_done)
  {
    already_done = true;

    fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    if (file_is_modified && !fl_choice("It seems that the subtitle file has been modified.\nDo you still want to quit without saving it ?", "No", "Yes", 0L))
    {
      already_done = false;
      return;
    }
    else
    {
      pref_set();
      delete main_window;
    }
  }
}

void quit_cb()
{
  quit_cb(nullptr, nullptr);
}

#ifdef _WIN32
std::string TxtError(DWORD gla)
{
  char *msg;
  DWORD len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, gla, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);

  if (len > 0)
  {
    msg[len - 1] = '\0';
    std::string ret(msg);
    LocalFree(msg);
    return ret;
  }

  return "";
}

std::string TxtError()
{
  DWORD gla = GetLastError();
  return TxtError(gla);
}
#endif

std::string theme = "";
bool gui_mode = true;
options myopt;
std::string opt_level = "";

int main(int argc, char **argv)
{
  std::string file = "", ofilename = "";
  int x = -1, y = -1, w = -1, h = -1;
  bool modify_input = false;
  bool run_pre_proc = false;
  int pp_time_start = -1, pp_time_stop = -1, pp_dur_k = 0, pp_offs_start = 0, pp_offs_stop = 0;

  myopt.set(argc, argv,
            {
                option_info(""),
                option_info(
                    'f', "file", [&](s_opt_params &p) -> void { file = p.val; }, "Name of the file to read. It is the same than directly passing a file name as an argument without this option.", required),

                option_info(
                    'g', "gui-mode", [&](s_opt_params &) -> void { gui_mode = true; }, "Process the input file and show it with the gui, this is the default behavior."),
                option_info(
                    'c', "batch-mode", [&](s_opt_params &) -> void { gui_mode = false; }, "Process the input the file and print the result."),

                option_info(""),
                option_info(
                    'b', "begin-time",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_time_start = str_to_ms(p.val);
                    },
                    "Change the beginning time stamp to the provided argument.", required),
                option_info(
                    'e', "end-time",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_time_stop = str_to_ms(p.val);
                    },
                    "Change the end time stamp to the provided argument.", required),
                option_info(
                    'k', "duration-coeff",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_dur_k = std::stoi(p.val);
                    },
                    "Change the duration coefficient to the provided argument.", required),
                option_info(
                    'a', "start-offset",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_offs_start = stoi(p.val);
                    },
                    "Change the start offset to the provided argument.", required),
                option_info(
                    's', "stop-offset",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_offs_stop = stoi(p.val);
                    },
                    "Change the stop offset to the provided argument.", required),
                option_info("These 5 previous options are processed after reading the file and have effect in both GUI and batch mode."),

                option_info(""),
                option_info(
                    'o', "output-file", [&](s_opt_params &p) -> void { ofilename = p.val; }, "Write the processing result into the file whose name is passed as argument.", required),
                option_info(
                    'i', "modify-input", [&](s_opt_params &) -> void { modify_input = true; }, "Write the processing result into the same input file."),
                option_info("These 2 previous options only have meaning in batch mode, they are ignored in GUI mode."),

                option_info(""),
                option_info(
                    'r', "reset-pref", [&](s_opt_params &) -> void { pref_reset(); }, "Reset the preferences to default values."),
                option_info(
                    'x', "xpos", [&](s_opt_params &p) -> void { x = std::stoi(p.val); }, "Set the x origin of the subadjust window.", required),
                option_info(
                    'y', "ypos", [&](s_opt_params &p) -> void { y = std::stoi(p.val); }, "Set the y origin of the subadjust window.", required),
                option_info(
                    'w', "width", [&](s_opt_params &p) -> void { w = std::stoi(p.val); }, "Set the width of the subadjust window.", required),
                option_info(
                    'h', "height", [&](s_opt_params &p) -> void { h = std::stoi(p.val); }, "Set the height of the subadjust window.", required),
                option_info(
                    't', "theme", [](s_opt_params &p) -> void { theme = p.val; }, R"EOF(Set the graphic theme to use. It is a string to choose between one of :
    classic, aero, metro, aqua, greybird, ocean, blue, olive, rose_gold, dark, brushed_metal or high_contrast.)EOF",
                    required),
                option_info(R"EOF(These 5 previous options only have effect in GUI mode. In this case, they have precedence and will update what is defined in the configuration file.
The configuration file is located there : ")EOF" +
                            std::filesystem::path(pref_filename()).make_preferred().string() + "\"."),

                option_info(""),
                option_info(
                    'l', "log-level",
                    [&](s_opt_params &p) -> void {
                      std::transform(p.val.begin(), p.val.end(), p.val.begin(), ::toupper);
                      eLogLevel ll = get_eloglev(p.val);
                      if (ll != LEVEL_UNDEFINED)
                      {
                        my_setenv("LOG", p.val);
                        opt_level = p.val;
                      }
                      // std::cout << ll << ", " << my_getenv("LOG") << std::endl;
                    },
                    R"EOF(Set the level of the log messages to display :
    ALL   All the messages.
    TRACE Almost all messages, at least those finer than the INFO level.
    INFO  Informational messages that highlight the application's progress at a coarser level.
    DEBUG Fine-grained events, the most useful for debugging an application.
    WARN  Potentially dangerous situations.
    ERROR Errors that might still allow the application to continue running.
    FATAL Very serious errors that will likely cause the application to crash.
    OFF   Disables logging.)EOF",
                    required),
                option_info(
                    'm', "log-file", [&](s_opt_params &p) -> void { my_setenv("LOGFILE", p.val); }, R"EOF(Define the file where log messages will be stored.
    Default it to store them in the following file )EOF" + DEF_LOG.string() + "\n    The special value 'console' will allows to output the log messages to the console, if possible.",
                    required),
                //                option_info(R"EOF(These 2 previous options have precedence on the environments variable LOG and LOGFILE.
                option_info(R"EOF(If none of these are defined, the default is to send the WARN and following log messages into the file ")EOF" + DEF_LOG.string() + "\"."),
                // option_info(""),
            });

  myopt.set_desc("A tool that allows to process subtitles files.\nThe batch mode allows processing at the command line or by script.\nMeanwhile the GUI mode adds a search and replace feature with regular expressions.");

  // Calls to logFunctions before opt.parse may not work correctly ...
  myopt.parse();

  // Text objects (file path and content)
  if (myopt.args.size() > 0)
  {
    logD("args");
    file = myopt.args[0];
  }

  if (gui_mode)
  {
    if (!ofilename.empty())
    {
      logW("Silently ignoring -o/-output-file in GUI mode");
      ofilename = "";
    }

    if (modify_input)
    {
      logW("Silently ignoring -i/-modify-input in GUI mode");
      modify_input = false;
    }

    // Main window
    make_window();
    fl_message_title_default("SubAdjust");
    main_window->icon(&svg);
    //  main_window->wait_for_expose();
    Fl::scrollbar_size(14);
    pref_get(x, y, w, h);

    fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);

    file_content->buffer(&txt_buf);
    file_content->linenumber_format("%05d");
    file_content->linenumber_width(50);

    txt_buf.add_modify_callback(file_modified, nullptr);

    if (theme != "")
      if (!OS::use_theme(theme))
        theme = "";

    //  } else {
    txt_buf.transcoding_warning_action = [](Fl_Text_Buffer *t) -> void {
      if (t->input_file_was_transcoded)
        logW("Displayed text contains the UTF-8 transcoding of the input file which was not UTF-8 encoded. Some changes may have occurred.");
    };
  }

  bool file_read_ok = file_read(file);

  if (run_pre_proc && file_read_ok && csub.vec().size() > 0)
    pre_process(pp_time_start, pp_time_stop, pp_offs_start, pp_offs_stop, pp_dur_k);

  if (gui_mode)
  {
    gui_display(file_read_ok);
    main_window->show();

    // Pref
    app_prefs->callback(SIMPLE_CB {
#ifdef NO_CONFIG_DLG
      std::string pf = std::filesystem::path(pref_filename()).make_preferred().string();
      logD("pf: ", pf);
      const std::string msg = "To do ...\nBut you may consider editing the following file:\n" + pf + "\nDo you want to proceed ?";
      if (fl_choice("%s", "No", "Yes", 0L, msg.c_str()))
      {
#ifdef _WIN32
        //      if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) >= 0)
        //      {
        std::wstring stemp = L"\"" + std::wstring(pf.begin(), pf.end()) + L"\"";
        if ((INT_PTR)ShellExecute(nullptr, L"edit", stemp.c_str(), nullptr, nullptr, SW_SHOWNORMAL) < 32)
          logD(std::to_string(GetLastError()));
//        CoUninitialize();
//      }
#else
      std::string edit("\""+std::filesystem::path("gvim").make_preferred().string()+"\" "+pf+" &");
      logD("edit: ", edit);
      std::system(edit.c_str());
#endif
      }
#else
      pref_dialog();
#endif
    });

    // About
    app_about->callback(about_msg, (void *)&myopt);

    // file features
    // file_open->callback();
    file_open->callback(SIMPLE_CB {
      // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
      if (file_is_modified && !fl_choice("Your actual changes will be lost. Do you still want to open another file?", "No", "Yes", 0L))
        return;

      file_handler();
    });

    file_save->callback(SIMPLE_CB { srt_save(); });
    file_save_as->callback(SIMPLE_CB { file_handler(false); });
    file_reload->callback(SIMPLE_CB {
      std::filesystem::path abs_path("");
      if (file_path->value() != nullptr)
        abs_path = std::filesystem::absolute(file_path->value());

      // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
      if (!abs_path.empty() && (!file_is_modified || fl_choice("It seems that the subtitle file has been modified.\nDo you still want to reload it ?", "No", "Yes", 0L)))
      {
        gui_display(file_read(abs_path), false);
      }
      // sub_adjust->deactivate();
    });

    app_save_quit->callback(SIMPLE_CB {
      if (srt_save())
        std::exit(0);
    });
    app_quit->callback(quit_cb);
    main_window->callback(quit_cb);
    std::at_quick_exit(quit_cb);
    std::atexit(quit_cb);

    // Go to features
    goto_start->callback(SIMPLE_CB {
      to_line(1);
      Fl::focus(file_content);
    });
    goto_end->callback(SIMPLE_CB {
      to_line(-100);
      Fl::focus(file_content);
    });
    goto_middle_line->callback(SIMPLE_CB {
      to_line(-50);
      Fl::focus(file_content);
    });
    goto_line->callback(SIMPLE_CB {
      logD("focus: ", Fl::focus(), ", goto_line:", goto_line, ", cb gtl: ", goto_line->value());
      to_line((int)goto_line->value());
      Fl::focus(file_content);
    });

    goto_middle_sub->callback(SIMPLE_CB {
      to_sub(-50);
      Fl::focus(file_content);
    });
    goto_middle_time->callback(SIMPLE_CB {
      to_time(-50);
      Fl::focus(file_content);
    });

    // Edit features :
    // Buttons : del_first_sub, del_last_sub
    del_first_sub->callback(delete_first_sub);
    del_last_sub->callback(delete_last_sub);

    goto_sub->callback(SIMPLE_CB { to_sub((int)goto_sub->value()); });

    time_to_go->callback(SIMPLE_CB {
      logD("time_to_go->get_time_ms(): ", time_to_go->get_time_ms());
      to_time(time_to_go->get_time_ms());
    });

    // Buttons find/replace and Inputs : str_find & str_replace
    find->callback(re_find);
    replace_next->callback(re_replace_next);
    replace_all->callback(re_replace_all);

    sub_adjust->callback(subadjust);
    sub_reparse->callback(reparse);

    to_line(1);
    Fl::flush();
    return Fl::run();
  }
  else
  {
    if (modify_input)
      ofilename = file;

    if (ofilename.empty())
      cui_display(file_read_ok, std::cout);
    else
    {
      std::ofstream ofs(ofilename);
      if (ofs)
      {
        cui_display(file_read_ok, ofs);
        ofs.close();
      }
      else
        std::cerr << "Unable to open file " << ofilename << std::endl;
    }
  }
}
