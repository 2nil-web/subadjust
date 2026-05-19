#ifndef FL_BROWSER_KEYBOARD_H
#define FL_BROWSER_KEYBOARD_H

#include <functional>
// To avoid warning C5260 in FL/Enumerations.H with Visual Studio.
// See from line 989 and around 70 that follow in the Enumeration.h file of fltk 1.4
#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 5260)
#include <FL/Fl.H>
#pragma warning(pop)
#endif
#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include "log.h"

// Classe template qui ajoute un handler personnalisable à n'importe quelle classe héritière de Fl_Browser_
template <typename Base> class Fl_Browser_Keyboard : public Base
{
  static_assert(std::is_base_of<Fl_Browser_, Base>::value, "Base must derive from Fl_Browser_");

private:
public:
  template <typename... Args>
  //    Fl_Browser_Keyboard(Args &&...args) : Base(std::forward<Args>(args)...)
  Fl_Browser_Keyboard(int x, int y, int w, int h, const char *label = 0L) : Base(x, y, w, h, label)
  {
  }

  int handle(int ev)
  {
    switch (ev)
    {
    case FL_KEYUP:
      const char c = std::toupper(*Fl::event_text());
      void *i = Base::selection();
      if (i == nullptr)
        i = Base::item_first();
      else
        i = Base::item_next(i);

      char c2 = std::toupper(*Base::item_text(i));
      if (c < c2)
        i = Base::item_first();

      for (; i; i = Base::item_next(i))
      {
        c2 = std::toupper(*Base::item_text(i));

        if (c == c2)
        {
          Base::select_only(i, 1);
          break;
        }
      }
      return 1;
    }

    return Base::handle(ev);
  }
};

// Alias
using Fl_Browser_Key = Fl_Browser_Keyboard<Fl_Browser>;
using Fl_Check_Browser_Key = Fl_Browser_Keyboard<Fl_Check_Browser>;
using Fl_File_Browser_Key = Fl_Browser_Keyboard<Fl_File_Browser>;
using Fl_Hold_Browser_Key = Fl_Browser_Keyboard<Fl_Hold_Browser>;
using Fl_Multi_Browser_Key = Fl_Browser_Keyboard<Fl_Multi_Browser>;
using Fl_Select_Browser_Key = Fl_Browser_Keyboard<Fl_Select_Browser>;

#endif /* FL_BROWSER_KEYBOARD_H */
