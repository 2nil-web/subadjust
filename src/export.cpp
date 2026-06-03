
#include "export.h"

// field1 is subtitle number
// field2 is subtitle start in HH:MM:SS.SSS format
// field3 is subtitle end in HH:MM:SS.SSS format
// field4 is subtitle start in floating point seconds
// field5 is subtitle start in floating point seconds
// field6 is subtitle text surrounded by text delimiter if requested (" or ')

std::vector<int> export_templ_parse(std::string sep, std::string delim)
{
  std::string s(export_template->value());
  std::vector<int> fsel = {};

  sep = trim(sep);
  std::vector<std::string> vf = split(s, sep[0]);
  for (auto token : vf)
  {
    token = trim(token);
    logD("Analyzing token1: ", token);

    if (token.substr(0, delim.size()) == delim)
    {
      token = token.substr(delim.size());
      size_t last_delim_pos = (token.find_last_of(delim));
      logD("Analyzing2 field: ", token);
      if (last_delim_pos != std::string::npos)
        token = token.substr(0, last_delim_pos);
    }

    logD("Analyzing token2: ", token);
    if (token.substr(0, 5) == "field")
    {
      int n = std::stoi(token.substr(5));
      logD("Pushing: ", n);
      if (n > 0 && n < 7)
        fsel.push_back(n);
      else
        logW("Wrong field index: ", n, ", ignoring it");
    }
    else
      logW("Wrong token: ", token, ", ignoring it");
  }

  return fsel;
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

  char dec_sep = '.';
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
  std::vector<int> fsel = export_templ_parse(vs, delim);

  if (fsel.empty())
    logD("export_to_file - fsel empty");
  else
    for (auto idx : fsel)
    {
      logD("export_to_file - resulting index: ", idx);
    }

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

static std::vector<Fl_Button *> rbseps;
void rbseps_cb(Fl_Widget *w, void *)
{
  for (auto rb : rbseps)
  {
    if ((Fl_Widget *)rb != w)
      rb->value(0);
  }
}

void only_one_char(Fl_Widget *w, void *)
{
  Fl_Input *inp = (Fl_Input *)w;
  std::string s = inp->value();
  if (s.size() > 1)
    s = s.back();
  if (isalnum(s[0]))
    inp->value("");
  else
    inp->value(s.c_str());
}

void def_one_char(Fl_Input *in)
{
  in->when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED | FL_WHEN_RELEASE);
  in->callback(only_one_char);
}

void export_run(Fl_Widget *, void *)
{
  static bool export_first_show = true;

  if (export_first_show)
  {
    extern Fl_SVG_Image *app_icon; // Defined and populated in subadjust.cpp
    export_dialog->icon(app_icon);

    export_semicolon->value(1);
    export_delims->select(1, true);
    export_bom->value(1);
    export_force_dec_sep->value(0);

    rbseps = {export_semicolon, export_tabulation, export_comma, export_other_sep};
    for (auto rbsep : rbseps)
    {
      rbsep->callback(rbseps_cb);
    }

    def_one_char(export_other_sep_val);
    def_one_char(export_dec_sep_val);

    export_field_sel->add(_("Subtitle number|Start time (HH:MM:SS.SSS)|End time(HH:MM:SS.SSS)|Start time (MSec)|End time (MSec)|Subtitle text"));
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
    export_first_show = false;
  }
  export_dialog->show();
}
