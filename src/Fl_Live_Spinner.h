#ifndef FL_LIVE_SPINNER_H
#define FL_LIVE_SPINNER_H

// To avoid warning C5260 in FL/Enumerations.H with Visual Studio.
// See from line 989 and around 70 that follow in the Enumeration.h file of fltk 1.4
#ifdef _MSVC_LANG
#pragma warning(push)
#pragma warning(disable : 5260)
#include <FL/Fl.H>
#pragma warning(pop)
#endif
#include <FL/Fl_Spinner.H>

// Like Fl_Spinner but adding event FL_WHEN_CHANGED to have live update without having to leave focus
class Fl_Live_Spinner : public Fl_Spinner
{
public:
  Fl_Live_Spinner(int x, int y, int w, int h, const char *label = nullptr) : Fl_Spinner(x, y, w, h, label)
  {
    input_.when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED | FL_WHEN_RELEASE);
  }
};
#endif /* FL_LIVE_SPINNER_H */
