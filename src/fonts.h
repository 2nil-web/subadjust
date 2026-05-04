#ifndef FONTS_H
#define FONTS_H

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "log.h"
#include "subadjust_ui.h"
#include "utils.h"

struct sfont_info
{
  Fl_Font number;
  int attr;
  std::string name, sattr;
  bool any_size = true;
  std::vector<int> sizes;
};

Fl_Font ensure_only_one_call_to_set_fonts();
const std::vector<std::string>& get_font_names_cache();
void get_fonts_info(std::vector<sfont_info> &fis, bool only_normal, bool only_any_size);
std::string fonts_info_string(bool only_normal = false, bool only_any_size = false);
void show_font_selector();

#endif /* FONTS_H */
