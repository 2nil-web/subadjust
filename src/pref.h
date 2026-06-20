#ifndef PREFS_H
#define PREFS_H
#include <filesystem>
#include <string>
#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#define DEF_WIN_X 80
#define DEF_WIN_Y 30
#define DEF_WIN_W 384
#define DEF_WIN_H 1000

#define MIN_X 0
#define MIN_Y 30
#define MIN_W DEF_WIN_W
#define MIN_H 600

#define SIMPLE_CB [](Fl_Widget *, void *)->void

void setup_i18n(std::filesystem::path, bool anticipate_dir = true);
std::string pref_get_string(const std::string, const std::string, Fl_Preferences *pref = nullptr);
int pref_get_int(const std::string, int, Fl_Preferences *pref = nullptr);
void pref_reset();
void pref_set();
void pref_get(int, int, int, int);
void pref_dialog(Fl_Widget *, void *);
void case_find(Fl_Widget *w = nullptr, void *v = nullptr);
const std::string pref_filename();
void font_redraw(std::string, int, int);
#endif /* PREFS_H */
