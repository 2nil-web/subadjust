
#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <regex>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/fl_ask.H>
#include <FL/platform.H>

#include "edit_features.h"
#include "file_features.h"
#include "log.h"
#include "options.h"
#include "subadjust_ui.h"
#include "subs.h"
#include "themes.h"
#include "utils.h"

extern options myopt; // Pour récupérer progname

Fl_Text_Buffer txt_buf;
bool file_is_modified = false;
cSub csub;

void set_file_state(bool modified)
{
  file_is_modified = modified;

  goto_line->maximum((double)csub.line_number());
  logT("goto_line->maximum: ", csub.line_number());

  double ns = (double)csub.vec().size();
  if (goto_sub->maximum() != ns && ns > 0)
  {
    goto_sub->maximum(ns);
    logT("goto_sub->maximum: ", ns);
  }

  if (modified)
  {
    logD("Modified put in red");
    //    if (file_path->labelcolor() != FL_RED)
    //    {
    file_path_label->labelcolor(FL_RED);
    ((Fl_Input_ *)file_path)->textcolor(FL_RED);
    file_path_label->redraw();
    file_path->redraw();
    file_path->redraw_label();
    main_window->redraw();
    //    }
  }
  else
  {
    logD("Not modified put in black");
    //    if (file_path->labelcolor() != FL_BLACK)
    //    {
    file_path_label->labelcolor(FL_BLACK);
    ((Fl_Input_ *)file_path)->textcolor(FL_BLACK);
    file_path_label->redraw_label();
    file_path->redraw_label();
    main_window->redraw();
    //    }
  }
}

bool run_set_file_state;
void has_to_set_file_state(bool modified)
{
  run_set_file_state = modified;
}

void do_set_file_state()
{
  set_file_state(run_set_file_state);
}

// void file_modified(int pos, int nInserted, int nDeleted, int nRestyled, const char *deletedText, void *)
void file_modified(int, int nInserted, int nDeleted, int, const char *, void *)
{
  // logT("pos: ", pos, ", nInserted: ", nInserted, ", nDeleted: ", nDeleted, ", nRestyled: ", nRestyled);
  // if (deletedText) logT(", deletedText: ", deletedText);
  if (nInserted != 0 || nDeleted != 0)
    set_file_state(true);
}

// $USERPROFILE/.subadjust_admin/already_opened || $HOME/already_opened
const std::filesystem::path already_opened_list(admin_file("already_opened"));

void remove_opened(std::filesystem::path abs_path)
{
  //  std::string abs_path(file_path->value());
  logD("remove_opened, to remove file: [", abs_path, ']');
  std::string line;
  std::vector<std::string> all_files;
  bool to_update = false;

  std::ifstream ifs(already_opened_list);
  while (std::getline(ifs, line))
  {
    trim(line);

    if (line == abs_path && !to_update)
    {
      to_update = true;
    }
    else
    {
      all_files.push_back(line);
    }
  }
  ifs.close();

  if (to_update)
  {
    logD("from: " + already_opened_list.string() + ", all_files.size(): ", all_files.size());
    std::ofstream ofs(already_opened_list, std::ios::trunc);
    for (std::string file : all_files)
    {
      logD("Update already_opened: ", file);
      ofs << file << std::endl;
    }
    ofs.close();
  }
}

void remove_opened(bool all)
{
  if (all)
    std::filesystem::remove(already_opened_list);
  else
  {
    static bool already_done = false;
    if (already_done)
      return;
    already_done = true;

    // logD("remove_opened");
    if (file_path->value() != nullptr)
      remove_opened(file_path->value());
  }
}

bool already_opened(std::string filename)
{
  if (trim(filename).empty())
    return false;

  std::string abs_path = std::filesystem::absolute(filename).string();
  logD("In already_opened, already_opened_list: [", already_opened_list, "], abs_path: [", abs_path, ']');

  // Simplest case
  if (!std::filesystem::exists(already_opened_list) || std::filesystem::file_size(already_opened_list) == 0)
  {
    std::ofstream ofs(already_opened_list);
    logD("Not already opened, Simply adding abs_path: ", abs_path);
    ofs << abs_path << std::endl;
    ofs.close();
    return false;
  }

  // already_opened_list exists and is not empty
  std::ifstream ifs(already_opened_list);
  std::string line;
  while (std::getline(ifs, line))
  {
    trim(line);
    // already opened
    if (line == abs_path)
    {
      ifs.close();
      logD("Already opened - line: ", line, ", abs_path: ", abs_path);
      return true;
    }
  }
  ifs.close();

  logD("Not already opened, adding abs_path: ", abs_path);
  std::ofstream ofs(already_opened_list, std::ios::app);
  ofs << abs_path << std::endl;
  ofs.close();

  return false;
}

bool file_read(const char *pfile)
{
  std::string filename = "";
  if (pfile)
    filename = std::string(pfile);
  return file_read(filename);
}

bool file_read(std::string filename)
{
  return file_read(std::filesystem::path(trim(filename)));
}

std::filesystem::path current_abs_path;
bool file_read(std::filesystem::path abs_path)
{
  if (abs_path.empty())
  {
    return false;
  }

  if (!abs_path.is_absolute())
    abs_path = std::filesystem::absolute(abs_path);

  if (txt_buf.loadfile(abs_path.string().c_str()) == 0)
  {
    bool diff = csub.diff(txt_buf.text());

    if (txt_buf.input_file_was_transcoded || diff)
    {
      if (diff)
        txt_buf.text(csub.c_str());
      logT("txt_buf.length(): ", txt_buf.length());
      has_to_set_file_state(true);
    }
    else
    {
      has_to_set_file_state(false);
    }

    if (csub.vec().size() > 0)
    {
      logD("csub.size(): ", csub.line_number(), ", csub.str().size(): ", csub.str().size(), ", csub.vec().size(): ", csub.vec().size());
      logD("csub.vec()[0].appearance: ", ms_to_str(csub.vec()[0].appearance), ", csub.vec().back().appearance: ", ms_to_str(csub.vec().back().appearance));
    }

    current_abs_path = abs_path;
    return true;
  }

  return false;
}

bool file_write(std::string filename)
{
  bool diff = csub.diff(txt_buf.text());
  logT("After parse file_write: ", (diff ? "" : "no "), "change");

  if (diff)
    txt_buf.text(csub.c_str());

  int errn = txt_buf.savefile(filename.c_str());
  logD("file_write err?:", errn);
  if (errn == 0)
  {
    set_file_state(false);
    return true;
  }
  else
  {
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    fl_alert("Error while saving file '%s'", filename.c_str());
    return false;
  }
}

void pre_process(int pp_time_start, int pp_time_stop, int pp_offs_start, int pp_offs_stop, int pp_dur_k)
{
  if (pp_time_start == -1)
    pp_time_start = csub.vec()[0].appearance;
  if (pp_time_stop == -1)
    pp_time_stop = csub.vec().back().appearance;
  if (pp_dur_k == 0)
    pp_dur_k = 1;
  logD("pp_time_start: ", pp_time_start, ", pp_time_stop: ", pp_time_stop, ", pp_offs_start: ", pp_offs_start, ", pp_offs_stop: ", pp_offs_stop, ", pp_dur_k: ", pp_dur_k);

  if (csub.adjust(pp_time_start, pp_time_stop, pp_offs_start, pp_offs_stop, pp_dur_k))
  {
    txt_buf.text(csub.c_str());
    has_to_set_file_state(true);
  }
  else
    has_to_set_file_state(false);
}

void gui_display(bool file_read_ok, bool test_already_opened)
{
  if (file_read_ok)
  {
    offset_start->value(0);
    offset_stop->value(0);
    dur_coeff->value(1);

    if (test_already_opened && already_opened(current_abs_path.string()))
    {
      if (!fl_choice("This file is already opened by another instance of this tool.\nDo you want to reopen it ?", "No", "Yes", 0L))
      {
        txt_buf.text("");
        set_file_state(false);
        return;
      }

      // Add the file twice in the already opened list ...
      if (!current_abs_path.empty())
      {
        std::ofstream ofs(already_opened_list, std::ios::app);
        // logD("Twice add: ", abs_path.string());
        ofs << current_abs_path.string() << std::endl;
        ofs.close();
      }
    }

    file_path->value(std::filesystem::absolute(current_abs_path).string().c_str());
    file_path->insert_position((int)current_abs_path.string().size());

    txt_buf.transcoding_warning_action = nullptr;
    do_set_file_state();

    std::string old_t = "";
    if (main_window->label())
      old_t = std::string("old title: ") + main_window->label() + ", ";
    std::string title = myopt.Progname + " - " + current_abs_path.stem().string();
    logD("TITLE: ", old_t, title);
#ifdef _WIN32
    main_window->label(title.c_str());
#else
    char t[1024];
    strncpy(t, title.c_str(), 1024);
    main_window->label(t);
#endif

    file_content->scroll(1, 0);
    to_line(1);
    file_content->show_cursor(1);
    if (csub.vec().size() > 0)
    {
      logD("csub.size(): ", csub.line_number(), ", csub.str().size(): ", csub.str().size(), ", csub.vec().size(): ", csub.vec().size());
      logD("csub.vec()[0].appearance: ", ms_to_str(csub.vec()[0].appearance), ", csub.vec().back().appearance: ", ms_to_str(csub.vec().back().appearance));
      time_start->set_time_ms(csub.vec()[0].appearance);
      time_end->set_time_ms(csub.vec().back().appearance);
    }
  }
  else
  {
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    if (!current_abs_path.empty())
      fl_alert("Unable to load the file '%s'", current_abs_path.string().c_str());
  }
}

void cui_display(bool file_read_ok, std::ostream &ofs)
{
  if (file_read_ok)
  {
    ofs << txt_buf.address(0) << std::flush;
  }
  else
  {
    std::cerr << "Unable to load the file " << current_abs_path << std::endl;
  }
}

bool file_handler(bool for_read)
{
  Fl_Native_File_Chooser fsel;
  int opts = Fl_Native_File_Chooser::Option::PREVIEW;
  int typ;
  std::string title;

  if (for_read)
  {
    typ = Fl_Native_File_Chooser::Type::BROWSE_FILE;
    title = "Pick file to load";
  }
  else
  {
    opts |= Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM | Fl_Native_File_Chooser::Option::NEW_FOLDER | Fl_Native_File_Chooser::Option::PREVIEW;
    typ = Fl_Native_File_Chooser::Type::BROWSE_SAVE_FILE;
    title = "Define file to save";
  }

  fsel.options(opts);
  fsel.title(title.c_str());
  fsel.type(typ);
  fsel.filter("Subtitle files\t*.srt");
  // fsel.directory("/var/tmp"); // default directory to use

  // Show file selector
  switch (fsel.show())
  {
  case -1:
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    fl_alert("%s", fsel.errmsg());
    break; // ERROR
  case 1:
    logT("CANCEL\n");
    break; // CANCEL
  default:
    logT("PICKED: %s\n", fsel.filename());
    if (for_read)
    {
      if (!current_abs_path.empty())
        remove_opened(current_abs_path);
      gui_display(file_read(fsel.filename()));
    }
    else
      file_write(fsel.filename());

    file_path->value(std::filesystem::absolute(fsel.filename()).string().c_str());

    return true; // FILE CHOSEN
  }

  return false;
}

bool srt_save()
{
  if (file_path->value() == nullptr)
    return file_handler(false);
  std::string filename = file_path->value();
  if (filename.empty())
    return file_handler(false);
  return file_write(filename);
}
