
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
    //logD("Modified put in red");
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
    //logD("Not modified put in black");
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
  //logD("remove_opened, to remove file: [", abs_path, ']');
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
    //logD("from: " + already_opened_list.string() + ", all_files.size(): ", all_files.size());
    std::ofstream ofs(already_opened_list, std::ios::trunc);
    for (std::string file : all_files)
    {
      //logD("Update already_opened: ", file);
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
  //logD("In already_opened, already_opened_list: [", already_opened_list, "], abs_path: [", abs_path, ']');

  // Simplest case
  if (!std::filesystem::exists(already_opened_list) || std::filesystem::file_size(already_opened_list) == 0)
  {
    std::ofstream ofs(already_opened_list);
    //logD("Not already opened, Simply adding abs_path: ", abs_path);
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
      //logD("Already opened - line: ", line, ", abs_path: ", abs_path);
      return true;
    }
  }
  ifs.close();

  //logD("Not already opened, adding abs_path: ", abs_path);
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
  //logD("file_read1 start", abs_path);

  if (abs_path.empty())
  {
    //logD("file_read false");
    return false;
  }

  if (!abs_path.is_absolute())
    abs_path = std::filesystem::absolute(abs_path);

  Fl_Text_Buffer txt_tmp;
  if (txt_tmp.loadfile(abs_path.string().c_str()) == 0)
  {
    cSub csub_tmp;
    csub_tmp.parse(txt_tmp.text());
    txt_buf.text(csub_tmp.c_str());

    if (txt_tmp.input_file_was_transcoded)
    {
      txt_buf.input_file_was_transcoded = true;
      // logT("txt_buf.length(): ", txt_buf.length());
      has_to_set_file_state(true);
    }
    else
    {
      txt_buf.input_file_was_transcoded = false;
      has_to_set_file_state(false);
    }

    current_abs_path = abs_path;
    //logD("file_read true: [", abs_path, "]");
    return true;
  }

  return false;
}

bool file_write(std::filesystem::path filename)
{
  csub.parse(txt_buf.text());
  txt_buf.text(csub.c_str());
  int errn=0;

  if (filename.extension() == ".srt")
    errn = txt_buf.savefile(filename.string().c_str());
  else if (filename.extension() == ".vtt")
  {
    Fl_Text_Buffer txt_vtt;
    txt_vtt.text(csub.to_vtt().c_str());
    errn = txt_vtt.savefile(filename.string().c_str());
  }
  else if (filename.extension() == ".csv")
  {
    Fl_Text_Buffer txt_csv;
    txt_csv.text(csub.to_csv().c_str());
    errn = txt_csv.savefile(filename.string().c_str());
  }

  //logD("file_write err?:", errn);
  if (errn == 0)
  {
    reset_param();
    set_file_state(false);
    return true;
  }
  else
  {
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    fl_alert(_("Error while saving file '%ls'"), filename.c_str());
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
  //logD("pp_time_start: ", pp_time_start, ", pp_time_stop: ", pp_time_stop, ", pp_offs_start: ", pp_offs_start, ", pp_offs_stop: ", pp_offs_stop, ", pp_dur_k: ", pp_dur_k);

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
      if (!fl_choice(_("This file is already opened by another instance of this tool.\nDo you want to reopen it ?"), _("No"), _("Yes"), 0L))
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
    //logD("GUI TITLE: ", old_t, title);
    main_window->copy_label(title.c_str());

    csub.parse(txt_buf.text());
    //logD("aft call csub.parse - csub.str().size(): ", csub.str().size(), ", csub.vec().size(): ", csub.vec().size());

    file_content->scroll(1, 0);
    to_line(1);
    file_content->show_cursor(1);
    //logD("gui_display SIZE: ", csub.vec().size());
    if (csub.vec().size() > 0)
    {
      //logD("gui_display csub.size(): ", csub.line_number(), ", csub.str().size(): ", csub.str().size(), ", csub.vec().size(): ", csub.vec().size());
      //logD("gui_display csub.vec()[0].appearance: ", ms_to_str(csub.vec()[0].appearance), ", csub.vec().back().appearance: ", ms_to_str(csub.vec().back().appearance));
      time_start->set_time_ms(csub.vec()[0].appearance);
      time_end->set_time_ms(csub.vec().back().appearance);
    }
  }
  else
  {
    //logD("gui_display out file_read_ok");
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    if (!current_abs_path.empty())
      fl_alert((_("Unable to load the file") + std::string(" '%s'")).c_str(), current_abs_path.string().c_str());
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

bool file_handler(eHandlingType ht)
{
  Fl_Native_File_Chooser fsel;
  int opts = Fl_Native_File_Chooser::Option::PREVIEW;
  int typ;
  std::string title;

  std::filesystem::path path = {};
  if (file_path->value() != nullptr)
      path = std::filesystem::absolute(file_path->value());
  if (!path.empty() && path.has_extension()) path.replace_extension("");

  switch (ht)
  {
  case eHandlingType::WRITE:
    opts |= Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM | Fl_Native_File_Chooser::Option::NEW_FOLDER | Fl_Native_File_Chooser::Option::PREVIEW | Fl_Native_File_Chooser::Option::USE_FILTER_EXT;
    typ = Fl_Native_File_Chooser::Type::BROWSE_SAVE_FILE;
    title = _("Define file to save");
    if (!path.empty())
      fsel.preset_file(path.string().c_str());
    fsel.filter(_("Write to file\t*.srt\n*.vtt\n*.csv"));
    break;
  case eHandlingType::SYNC:
    typ = Fl_Native_File_Chooser::Type::BROWSE_FILE;
    title = _("Pick a file to use for synchronization");
    if (!path.empty())
      fsel.preset_file(path.parent_path().string().c_str());
    fsel.filter(_("Sync with file\t*.{srt,vtt}"));
    break;

  case eHandlingType::READ:
  default:
    typ = Fl_Native_File_Chooser::Type::BROWSE_FILE;
    title = _("Pick a file to load");
    if (!path.empty())
      fsel.preset_file(path.string().c_str());
    fsel.filter(_("Read file\t*.{srt,vtt}"));
    break;
  }

  fsel.options(opts);
  fsel.title(title.c_str());
  fsel.type(typ);
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
    path = std::filesystem::absolute(fsel.filename());
    if (!path.has_extension() || (path.extension() != ".srt" && path.extension() != ".vtt" && path.extension() != ".csv"))
    {
      switch (fsel.filter_value())
      {
      case 1:
        path+=".vtt";
        break;
      case 2:
        path+=".csv";
        break;
      case 0:
      default:
        path+=".srt";
        break;
      }
    }

    logD("file_handler dest path: ", path);
    switch (ht)
    {
    case eHandlingType::READ:
      if (!current_abs_path.empty())
        remove_opened(current_abs_path);
      current_abs_path = path;

      file_path->value(path.string().c_str());
      gui_display(file_read(path.string().c_str()));
      break;

    case eHandlingType::WRITE: {
      file_write(path);
      file_path->value(path.string().c_str());
    }
    break;

    case eHandlingType::SYNC:
      csub.sync_with = path;
      break;
    }

    return true; // FILE CHOSEN
  }

  return false;
}

bool native_save()
{
  if (file_path->value() == nullptr)
    return file_handler(eHandlingType::WRITE);
  std::string filename = file_path->value();
  if (filename.empty())
    return file_handler(eHandlingType::WRITE);
  return file_write(filename);
}
