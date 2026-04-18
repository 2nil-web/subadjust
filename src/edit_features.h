#ifndef EDIT_FEATURES_H
#define EDIT_FEATURES_H

#include <string>

#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/Fl_Widget.H>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#include "subs.h"
// #include "file_features.h"

// Expect an int that when positive, is a line index
// When negative, take its absolute value as a percentage of the line count
void to_line(int d);
// Expect an int that when positive, is a subtitle index
// When negative, take its absolute value as a percentage of the subtitle count
int to_sub(int d);
// Expect an int that when positive, is a time in milliseconds
// When negative, take its absolute value as a percentage of the subtitle duration
void to_time(int d);
bool is_valid_regex(std::regex &re);
bool find_pos1(std::string s, size_t &pos);
void re_find(Fl_Widget *, void *);
void re_replace(bool all = true);
void re_replace_next(Fl_Widget *, void *);
void re_replace_all(Fl_Widget *, void *);
void reparse(Fl_Widget *w = nullptr, void *v = nullptr);
void delete_first_sub(Fl_Widget *, void *);
void delete_last_sub(Fl_Widget *, void *);
#endif /* EDIT_FEATURES_H */
