
#include "export.h"

void export_run(Fl_Widget *, void *)
{
  cancel_export->callback(SIMPLE_CB { export_dialog->hide(); });
  export_dialog->show();
}
