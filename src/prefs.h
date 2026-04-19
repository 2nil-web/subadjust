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
void reset_prefs();
void set_prefs();
void get_prefs(int, int, int, int);
void case_find(Fl_Widget *w = nullptr, void *v = nullptr);
const std::string pref_filename();
#endif /* PREFS_H */
