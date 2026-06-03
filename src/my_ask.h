#ifndef MY_ASK_H
#define MY_ASK_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

extern Fl_SVG_Image *app_icon;

#include "Fl_Hover.H"
#include "options.h"
#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <string>
#include <vector>

void my_message_position(int, int);

void my_font(Fl_Font, Fl_Fontsize);
void my_fontsize(Fl_Fontsize);

void my_message(const char *);
int my_choice(const char *, ...);

#endif /* MY_ASK_H */
