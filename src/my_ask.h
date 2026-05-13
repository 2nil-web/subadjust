#ifndef MY_ASK_H
#define MY_ASK_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <cstdarg>
#include "Fl_Hover_Button.H"

void my_message(const char* msg, Fl_Font font = FL_HELVETICA, Fl_Fontsize size = 14);
int my_choice(Fl_Font font, Fl_Fontsize size, const char* msg, ...);

#endif /* MY_ASK_H */
