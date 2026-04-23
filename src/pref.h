#ifndef PREFS_H
#define PREFS_H
#include <string>
#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/Fl.H>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#define SIMPLE_CB [](Fl_Widget *, void *)->void

void pref_reset();
void pref_set();
void pref_get(int, int, int, int);
void pref_dialog();
void case_find(Fl_Widget *w = nullptr, void *v = nullptr);
const std::string pref_filename();
#endif /* PREFS_H */
