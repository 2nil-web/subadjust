
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>

#ifdef _WIN32
// #define WIN32_LEAN_AND_MEAN
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
#include "export.h"
#include "file_features.h"
#include "fonts.h"
#include "log.h"
#include "my_ask.h"
#include "options.h"
#include "pref.h"
#include "subadjust_icon.h"
#include "subadjust_ui.h"
#include "subs.h"
#include "themes.h"
#include "utils.h"

Fl_SVG_Image *app_icon;

void about_msg(Fl_Widget *, void *v)
{
  static bool once = true;

  if (once)
  {
    once = false;
    options *opt = (options *)v;
    std::string about_text = opt->version(Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R));

    // svg.scale(18, 18); fl_message_icon_label(""); fl_message_icon()->image(svg);
    my_message_position(main_window->x_root(), main_window->y_root() + 60);
    my_font(FL_HELVETICA, 14);
    // if (!fl_choice("%s", _("More info ..."), "OK", 0L, about_text.c_str()))
    if (my_choice(about_text.c_str(), "OK", _("More info ..."), nullptr) == 1)
    {
      my_font(FL_COURIER_BOLD, 10);
      my_message(opt->usage().c_str());
    }
    once = true;
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
  double offs_stop = get_reset_val(offset_stop, 0);
  double dur_k = get_reset_val(dur_coeff, 1);

  if (csub.adjust(beg_ts, end_ts, offs_start, offs_stop, dur_k))
  {
    time_start->set_time_ms(csub.vec()[0].appearance);
    time_end->set_time_ms(csub.vec().back().appearance);
    txt_buf.text(csub.c_str());
    file_content->redraw();
    set_file_state(true);
    if (csub.err_msg != "")
    {
      fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
      fl_message("%s", csub.err_msg.c_str());
    }
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
    if (file_is_modified && !fl_choice(_("It seems that the subtitles file has been modified.\nDo you still want to quit without saving it ?"), _("No"), _("Yes"), 0L))
    {
      already_done = false;
      return;
    }

    if (config_dialog->shown())
    {
      config_dialog->hide();
      delete config_dialog;
    }

    if (export_dialog->shown())
    {
      export_dialog->hide();
      delete export_dialog;
    }

    pref_set();
    delete main_window;
  }
}

void quit_cb()
{
  quit_cb(nullptr, nullptr);
}

std::string theme = "";
bool gui_mode = true;
options myopt;
std::string opt_level = "";

int main(int argc, char **argv)
{
  setup_i18n(std::filesystem::path(argv[0]).parent_path());

  std::filesystem::path ifile = "", ofilename = "";
  int x = -1, y = -1, w = -1, h = -1;
  bool modify_input = false;
  bool run_pre_proc = false;
  int pp_time_start = -1, pp_time_stop = -1, pp_dur_k = 0, pp_offs_start = 0, pp_offs_stop = 0;

  myopt.set(argc, argv,
            {
                option_info(""),
                option_info(
                    'g', "gui-mode", [&](s_opt_params &) -> void { gui_mode = true; }, _("Process the input file and show it with the gui, this is the default behavior.")),
                option_info(
                    'c', "batch-mode", [&](s_opt_params &) -> void { gui_mode = false; }, _("Process the input the file and print the result.")),
                option_info(
                    'f', "input-file", [&](s_opt_params &p) -> void { ifile = p.val; }, _("Name of the file to read. It is the same than directly passing a file name as an argument without this option."), required),

                option_info(
                    'o', "output-file",
                    [&](s_opt_params &p) -> void {
                      ofilename = p.val;
                      gui_mode = false;
                    },
                    _("Process the input file in batch mode and write the result into the file whose name is passed in argument.\n    Based on the file extension, may write it to SubRip (.srt), WEBVTT (.vtt) or even CSV (.csv) format."), required),
                option_info(
                    'i', "modify-input", [&](s_opt_params &) -> void { modify_input = true; }, _("Write the processing result into the same input file. Only has a meaning in batch mode, ignored in GUI mode.")),

                option_info(""),
                option_info(
                    'r', "reset-conf",
                    [&](s_opt_params &) -> void {
                      gui_mode = false;
                      pref_reset();
                    },
                    _("Reset the configuration and exit.")),

                option_info(""),
                option_info(
                    'b', "begin-time",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_time_start = str_to_ms(p.val);
                    },
                    _("Change the beginning time stamp to the provided argument."), required),
                option_info(
                    'e', "end-time",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_time_stop = str_to_ms(p.val);
                    },
                    _("Change the end time stamp to the provided argument."), required),
                option_info(
                    'k', "duration-coeff",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_dur_k = std::stoi(p.val);
                    },
                    _("Change the duration coefficient to the provided argument."), required),
                option_info(
                    'a', "start-offset",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_offs_start = stoi(p.val);
                    },
                    _("Change the start offset to the provided argument."), required),
                option_info(
                    's', "stop-offset",
                    [&](s_opt_params &p) -> void {
                      run_pre_proc = true;
                      pp_offs_stop = stoi(p.val);
                    },
                    _("Change the stop offset to the provided argument."), required),
                option_info(_("These 5 previous options are processed after reading the file and have effect in both GUI and batch mode.")),

                option_info(""),
                /*
                                option_info(
                                    'x', "xpos", [&](s_opt_params &p) -> void { x = std::stoi(p.val); }, _("Set the x origin of the subadjust window."), required),
                                option_info(
                                    'y', "ypos", [&](s_opt_params &p) -> void { y = std::stoi(p.val); }, _("Set the y origin of the subadjust window."), required),
                                option_info(
                                    'w', "width", [&](s_opt_params &p) -> void { w = std::stoi(p.val); }, _("Set the width of the subadjust window."), required),
                                option_info(
                                    'h', "height", [&](s_opt_params &p) -> void { h = std::stoi(p.val); }, _("Set the height of the subadjust window."), required),
                                option_info(
                                    't', "theme", [](s_opt_params &p) -> void { theme = p.val; }, _(R"EOF(Set the graphic theme to use. It is a string to choose between one of :
                    classic, aero, metro, aqua, greybird, ocean, blue, olive, rose_gold, dark, brushed_metal or high_contrast.)EOF"),
                                    required),
                                option_info(_(R"EOF(These 5 previous options only have effect in GUI mode. In this case, they have precedence and will update what is defined in the configuration file.
                The configuration file is located there : ")EOF") +
                                            std::filesystem::path(pref_filename()).make_preferred().string() + "\"."),
                                option_info(""),
                */
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
                    _(R"EOF(Set the level of the log messages to display :
    ALL   All the messages.
    TRACE Almost all messages, at least those finer than the INFO level.
    INFO  Informational messages that highlight the application's progress at a coarser level.
    DEBUG Fine-grained events, the most useful for debugging an application.
    WARN  Potentially dangerous situations.
    ERROR Errors that might still allow the application to continue running.
    FATAL Very serious errors that will likely cause the application to crash.
    OFF   Disables logging.)EOF"),
                    required),
                option_info(
                    'm', "log-file", [&](s_opt_params &p) -> void { my_setenv("LOGFILE", p.val); },
                    _("Define the file where log messages will be stored.\n    Default it to store them in ") + DEF_LOG.string() + _("\n    The special value 'console' will allows to output the log messages to the console, if possible."), required),
            });

  myopt.set_desc(_("A tool that allows to process subtitles text files.\nThe batch mode allows processing at the command line or by script.\nMeanwhile the GUI mode adds a search and replace feature with regular expressions."));

  // Calls to logFunctions before opt.parse may not work correctly ...
  myopt.parse();

  // Text objects (file path and content)
  if (myopt.args.size() > 0)
  {
    ifile = myopt.args[0];
    logD("args, file: ", ifile);
  }

  if (gui_mode)
  {
    if (!ofilename.empty())
    {
      logW("Silently ignoring -o/--output-file in GUI mode");
      ofilename = "";
    }

    if (modify_input)
    {
      logW("Silently ignoring -i/--modify-input in GUI mode");
      modify_input = false;
    }

    // Main window
    make_window();
    fl_message_title_default("SubAdjust");
    app_icon = new Fl_SVG_Image(nullptr, subadjust_svg_data);
    main_window->icon(app_icon);

    //  main_window->wait_for_expose();
    Fl::scrollbar_size(14);

    get_font_names_cache(); // ← force la capture des noms avant use_native_fonts()

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

  bool file_read_ok = file_read(ifile);

  if (run_pre_proc && file_read_ok && csub.vec().size() > 0)
    pre_process(pp_time_start, pp_time_stop, pp_offs_start, pp_offs_stop, pp_dur_k);

  if (gui_mode)
  {
    gui_display(file_read_ok);
    main_window->show();

    // Pref
    app_prefs->callback(pref_dialog);

    // About
    app_about->callback(about_msg, (void *)&myopt);

    // file features
    // file_open->callback();
    file_open->callback(SIMPLE_CB {
      logD("FILE OPEN1");
      fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
      if (file_is_modified && !fl_choice(_("Your actual changes will be lost. Do you still want to open another file?"), _("No"), _("Yes"), 0L))
      {
        return;
      }
      logD("FILE OPEN2");

      file_handler(eHandlingType::READ);
    });

    file_save->callback(SIMPLE_CB { save(); });
    file_save_as->callback(SIMPLE_CB { file_handler(eHandlingType::WRITE); });
    file_export->callback(export_run);

    file_reload->callback(SIMPLE_CB {
      std::string s = file_path->value();
      if (!s.empty())
      {
        std::filesystem::path abs_path = std::filesystem::absolute(s);
        fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
        if (!abs_path.empty() && (!file_is_modified || fl_choice(_("It seems that the subtitles file has been modified.\nDo you still want to reload it ?"), _("No"), _("Yes"), 0L)))
        {
          gui_display(file_read(abs_path), false);
        }
        // sub_adjust->deactivate();
      }
    });

    app_save_quit->callback(SIMPLE_CB {
      if (save())
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
    case_sensitive_find->callback(case_find);
    replace_next->callback(re_replace_next);
    replace_all->callback(re_replace_all);

    sub_adjust->callback(subadjust);
    sub_reparse->callback(reparse);
    sub_sync->callback(sync);

    to_line(1);
    std::string font_name = pref_get_string("font name", "");
    int font_num = pref_get_int("font number", FL_HELVETICA);
    int font_sz = pref_get_int("font size", 14);
    if (!font_name.empty() || font_num != FL_HELVETICA || font_sz != 14)
      font_redraw(font_name, font_num, font_sz);

    Fl::flush();
    return Fl::run();
  }
  else
  {
    if (modify_input)
      ofilename = ifile;

    if (ofilename.empty())
      cui_display(file_read_ok, std::cout);
    else
    {
#ifdef TOTO
      std::ofstream ofs(ofilename);
      if (ofs)
      {
        cui_display(file_read_ok, ofs);
        ofs.close();
      }
      else
      {
        logW(_("Unable to open file "), ofilename);
        std::cerr << _("Unable to open file ") << ofilename << std::endl;
      }
#else
      cui_display(file_read_ok, ofilename);
#endif
    }
  }
}
