
#include "Fl_Hover_Button.H"

Fl_Hover_Button::Fl_Hover_Button(int x, int y, int w, int h, const char *label) : Fl_Button(x, y, w, h, label)
{
  // default_color = fl_lighter(FL_BLUE);
  when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED | FL_WHEN_RELEASE);
}

int Fl_Hover_Button::handle(int e)
{
  int ret = Fl_Button::handle(e);
  switch (e)
  {
  case FL_ENTER:
    default_color = color();
    color(FL_YELLOW);
    redraw();
    return 1;
  case FL_LEAVE:
    color(default_color);
    redraw();
    return 1;
  }
  return ret;
}
