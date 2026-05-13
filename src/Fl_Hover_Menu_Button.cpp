
#include "Fl_Hover_Menu_Button.H"

Fl_Hover_Menu_Button::Fl_Hover_Menu_Button(int x, int y, int w, int h, const char *label) : Fl_Menu_Button(x, y, w, h, label)
{
  // default_color = fl_lighter(FL_BLUE);
  when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED | FL_WHEN_RELEASE);
}

int Fl_Hover_Menu_Button::handle(int e)
{
  static Fl_Color col = color(), lab_col = labelcolor();
  int ret = Fl_Menu_Button::handle(e);
  switch (e)
  {
  case FL_ENTER:
    color(FL_WHITE); // fl_rgb_color(0xFF, 0xFF, 0xEF));
    redraw();
    return 1;
  case FL_LEAVE:
    color(col);
    labelcolor(lab_col);
    redraw();
    return 1;
  }
  return ret;
}
