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
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>


// Like Fl_Button but adding event FL_WHEN_CHANGED to have live update without having to leave focus
// Classe template qui ajoute un handler personnalisable
// à n'importe quelle classe héritière de Fl_Widget
template<typename Base>
class Fl_Hover : public Base {
public:
  //void Fl_Widget::when 	( 	uchar 	i	) 	
  //  using WhenFn = std::function<void(Fl_Hover<Base>*, uchar)>;
  
    // Constructeurs variadiques pour passer les args à la classe Base
    template<typename... Args>
    Fl_Hover(Args&&... args) : Base(std::forward<Args>(args)...)
    {
      Base::when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED | FL_WHEN_RELEASE);
    }

    int handle(int ev) {
      // On gère ENTER, LEAVE et SHOW
      static Fl_Color col = Base::color(), lab_col = Base::labelcolor();

      switch (ev)
      {
      case FL_ENTER:
        Base::color(FL_WHITE); // fl_rgb_color(0xFF, 0xFF, 0xFF));
        Base::redraw();
        return 1;
      case FL_LEAVE:
      case FL_SHOW:
        Base::color(col);
        Base::labelcolor(lab_col);
        Base::redraw();
        return 1;
      }


      return Base::handle(ev);
    }
};

//using Fl_Hover_Button = Fl_Hover<Fl_Button>;
//using Fl_Hover_Menu_Button = Fl_Hover<Fl_Menu_Button>;

#endif /* FL_HOVER_H */

