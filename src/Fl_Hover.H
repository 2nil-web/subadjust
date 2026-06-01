#ifndef FL_HOVER_H
#define FL_HOVER_H

#include <functional>
// To avoid warning C5260 in FL/Enumerations.H with Visual Studio.
// See from line 989 and around 70 that follow in the Enumeration.h file of fltk 1.4
#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 5260)
#include <FL/Fl.H>
#pragma warning(pop)
#endif
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Shortcut_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Toggle_Light_Button.H>
#include <FL/Fl_Toggle_Round_Button.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

// Classe template qui ajoute un handler personnalisable à n'importe quelle classe héritière de Fl_Widget
template <typename Base> class Fl_Hover : public Base
{
  // static_assert(std::is_base_of<Fl_Button, Base>::value, "Base must derive from Fl_Button");
  // static_assert(std::is_base_of_v<Fl_Button, Base>, "Base must derive from Fl_Button");
private:
  bool first_show;

public:
  // Constructeurs variadiques pour passer les args à la classe Base
  template <typename... Args>
  //    Fl_Hover(Args &&...args) : Base(std::forward<Args>(args)...)
  Fl_Hover(int x, int y, int w, int h, const char *label = 0L) : Base(x, y, w, h, label)
  {
    first_show = true;
  }

  int handle(int ev)
  {
    static Fl_Color col = Base::color(), lab_col = Base::labelcolor();

    // On gère ENTER, LEAVE et SHOW
    switch (ev)
    {
    case FL_ENTER:
      Base::color(FL_WHITE);
      Base::redraw();
      return 1;
    case FL_LEAVE:
    case FL_SHOW:
      // Force certain attributs graphiques
      if (first_show)
      {
        Base::box(FL_BORDER_BOX);
        Base::color(FL_BACKGROUND_COLOR);
        Base::selection_color(FL_BACKGROUND_COLOR);
        Base::when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
        first_show = false;
      } else Base::color(col);
      Base::labelcolor(lab_col);
      Base::redraw();
      return 1;
    }

    return Base::handle(ev);
  }
};

// --- Alias pratiques pour les types courants ---
using Fl_Button_Hover = Fl_Hover<Fl_Button>;
using Fl_Check_Button_Hover = Fl_Hover<Fl_Check_Button>;
using Fl_Choice_Hover = Fl_Hover<Fl_Choice>;
using Fl_Light_Button_Hover = Fl_Hover<Fl_Light_Button>;
using Fl_Menu_Button_Hover = Fl_Hover<Fl_Menu_Button>;
using Fl_Radio_Button_Hover = Fl_Hover<Fl_Radio_Button>;
using Fl_Radio_Light_Button_Hover = Fl_Hover<Fl_Radio_Light_Button>;
using Fl_Radio_Round_Button_Hover = Fl_Hover<Fl_Radio_Round_Button>;
using Fl_Repeat_Button_Hover = Fl_Hover<Fl_Repeat_Button>;
using Fl_Return_Button_Hover = Fl_Hover<Fl_Return_Button>;
using Fl_Round_Button_Hover = Fl_Hover<Fl_Round_Button>;
using Fl_Shortcut_Button_Hover = Fl_Hover<Fl_Shortcut_Button>;
using Fl_Toggle_Button_Hover = Fl_Hover<Fl_Toggle_Button>;
using Fl_Toggle_Light_Button_Hover = Fl_Hover<Fl_Toggle_Light_Button>;
using Fl_Toggle_Round_Button_Hover = Fl_Hover<Fl_Toggle_Round_Button>;

#endif /* FL_HOVER_H */
