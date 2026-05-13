#ifndef FL_HOVER_BUTTON_H
#define FL_HOVER_BUTTON_H

// To avoid warning C5260 in FL/Enumerations.H with Visual Studio.
// See from line 989 and around 70 that follow in the Enumeration.h file of fltk 1.4
#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 5260)
#include <FL/Fl.H>
#pragma warning(pop)
#endif
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>

#include "log.h"

// Like Fl_Button but adding event FL_WHEN_CHANGED to have live update without having to leave focus
class Fl_Hover_Button : public Fl_Button
{
public:
  Fl_Hover_Button(int x, int y, int w, int h, const char *label = nullptr);
  int handle(int);
};
#endif /* FL_HOVER_BUTTON_H */

