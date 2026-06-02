
#include "export.h"

// field1 is subtitle number
// field2 is subtitle start in HH:MM:SS.SSS format
// field3 is subtitle end in HH:MM:SS.SSS format
// field4 is subtitle start in floating point seconds
// field5 is subtitle start in floating point seconds
// field6 is subtitle text surrounded by text delimiter if requested (" or ')
void export_to_file(std::string exp_templ, std::string sep, std::string delim)
{
  logD("export_to_file - sep: ", sep, ", delim: ", delim, ", exp_templ: ", exp_templ);
}

std::string get_sep()
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
      exp_templ += get_sep();
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

  save_export->callback(SIMPLE_CB { export_to_file(export_template->value(), get_sep(), export_delims->text(export_delims->value())); });

  close_export->callback(SIMPLE_CB { export_dialog->hide(); });
  export_dialog->show();
}
