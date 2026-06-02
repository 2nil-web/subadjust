
#include "export.h"

// field1 is subtitle number
// field2 is subtitle start in HH:MM:SS.SSS format
// field3 is subtitle end in HH:MM:SS.SSS format
// field4 is subtitle start in floating point seconds
// field5 is subtitle start in floating point seconds
// field6 is subtitle text surrounded by text delimiter if requested (" or ')

std::vector<bool> export_templ_parse(std::string sep, std::string delim)
{
  std::string s(export_template->value());
  std::vector<bool> fsel;
  if (s.empty())
    fsel = {true, true, true, true, true, true};
  else
  {
    sep=trim(sep);
    std::vector<std::string> vf=split(s, sep[0]);
    for (auto f:vf) {
      f=trim(f);
      if (f.substr(0, delim.size()) == delim) {
        f=f.substr(delim.size());
        size_t last_delim_pos=(f.find_last_of(delim));
        if (last_delim_pos != std::string::npos) f=f.substr(0, last_delim_pos);
 ////////////////////////
      }
    }
  }

  // return fsel;
  return {true, true, true, true, true, true};
}

std::string val_sep()
{
  std::string sep = "";
  if (export_comma->value() == 1)
    sep += ",";
  if (export_semicolon->value() == 1)
    sep += ";";
  if (export_tabulation->value() == 1)
    sep += "\t";
  if (export_other_sep->value() == 1)
    sep += export_other_sep_val->value();

  return sep;
}

void export_to_file(Fl_Widget *, void *)
{
  std::filesystem::path path = {};
  if (file_path->value() != nullptr)
  {
    path = file_path->value();
  }

  if (path.empty())
  {
    fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    fl_alert(_("Missing file path, save it before exporting it."));
    return;
  }

  if (txt_buf.length() == 0)
  {
    blink_title(_("<<<NOTHING TO SAVE>>>"));
    return;
  }

  csub.parse(txt_buf.text());

  if (csub.vec().size() == 0)
  {
    fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    fl_alert(_("File does not seem to be of subrip format\nAborting the save..."));
    return;
  }

  path = std::filesystem::absolute(file_path->value());
  if (path.has_extension())
    path.replace_extension("csv");

  std::string exp_templ(export_template->value());
  std::string vs = val_sep(), delim = {};
  if (export_delims->value() > -1)
  {
    delim = export_delims->text(export_delims->value());
  }

  char dec_sep = '\0';
  if (export_force_dec_sep->value() == 1)
  {
    std::string dec_seps = "";

    if (export_dec_sep_val->value() != nullptr)
      dec_seps = export_dec_sep_val->value();
    dec_seps = trim(dec_seps);
    if (dec_seps.size() > 0)
      dec_sep = dec_seps[0];
  }

  bool with_bom = (export_bom->value() == 1);

  logD("export_to_file - path: ", path, ", vs: ", vs, ", delim: ", delim, ", dec_sep: ", dec_sep, ", with_bom: ", with_bom, ", exp_templ: ", export_template->value());
  std::vector<bool> fsel = export_templ_parse(vs, delim);
  std::string buf = csub.to_sv(vs, delim, dec_sep, with_bom, fsel);
  if (!buf.empty())
  {
    Fl_Text_Buffer txt_tmp;
    txt_tmp.text(buf.c_str());
    if (txt_tmp.savefile(path.string().c_str()) != 0)
    {
      fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
      fl_alert(
#ifdef _WIN32
          _("Error while exporting file '%ls'")
#else
          _("Error while exporting file '%s'")
#endif
              ,
          path.c_str());
    }
  }
}

void export_run(Fl_Widget *, void *)
{
  static bool export_first_show = true;

  if (export_first_show)
  {
    export_semicolon->value(1);
    export_delims->select(1, true);
    export_bom->value(1);
    export_force_dec_sep->value(0);
    export_first_show = false;
  }

  export_field_add->callback(SIMPLE_CB {
    std::string exp_templ = export_template->value();
    if (!exp_templ.empty())
      exp_templ += val_sep();
    if (export_field_sel->value() > -1)
    {
      if (export_field_sel->value() == 5)
      {
        exp_templ += export_delims->text(export_delims->value());
        exp_templ += "field" + std::to_string(export_field_sel->value() + 1);
        exp_templ += export_delims->text(export_delims->value());
      }
      else
        exp_templ += "field" + std::to_string(export_field_sel->value() + 1);

      export_template->value(exp_templ.c_str());
    }
  });

  save_export->callback(export_to_file);

  close_export->callback(SIMPLE_CB { export_dialog->hide(); });
  export_dialog->show();
}
