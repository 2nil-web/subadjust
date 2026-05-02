//
// Font demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Fl_Double_Window *form, *font_sel_win;
Fl_Tile *tile;
Fl_Window *vector_font_editor = 0;

class FontDisplay : public Fl_Widget
{
  void draw() FL_OVERRIDE;

public:
  int font, size;
  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char *L = 0) : Fl_Widget(X, Y, W, H, L)
  {
    box(B);
    font = 0;
    size = 14;
  }
};
void FontDisplay::draw()
{
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(FL_BLACK);
  fl_draw(label(), x() + 3, y() + 3, w() - 6, h() - 6, align());
}

FontDisplay *textobj;

Fl_Hold_Browser *fontobj, *sizeobj;

int **sizes;
int *numsizes;
int pickedsize = 14;

void font_cb(Fl_Widget *, long)
{
  int fn = fontobj->value();
  if (!fn)
    return;
  std::cout << "SEL FONT:" << fontobj->value() << ", " << fontobj->text(fontobj->value()) << std::endl;
  fn--;
  textobj->font = fn;
  sizeobj->clear();
  int n = numsizes[fn];
  int *s = sizes[fn];
  if (!n)
  {
    // no sizes
  }
  else if (s[0] == 0)
  {
    // many sizes;
    int j = 1;
    for (int i = 1; i < 64 || i < s[n - 1]; i++)
    {
      char buf[20];
      if (j < n && i == s[j])
      {
        snprintf(buf, 20, "@b%d", i);
        j++;
      }
      else
        snprintf(buf, 20, "%d", i);
      sizeobj->add(buf);
    }
    sizeobj->value(pickedsize);
  }
  else
  {
    // some sizes
    int w = 0;
    for (int i = 0; i < n; i++)
    {
      if (s[i] <= pickedsize)
        w = i;
      char buf[20];
      snprintf(buf, 20, "@b%d", s[i]);
      sizeobj->add(buf);
    }
    sizeobj->value(w + 1);
  }
  textobj->redraw();
}

void size_cb(Fl_Widget *, long)
{
  int i = sizeobj->value();
  if (!i)
    return;
  const char *c = sizeobj->text(i);
  while (*c < '0' || *c > '9')
    c++;
  pickedsize = atoi(c);
  textobj->size = pickedsize;
  textobj->redraw();
}

char label[0x1000];

unsigned char current_char = 'A';
unsigned char vec[255][128] = {{0}};

void create_font_display()
{
  // create the sample string
  int n = 0;
  strcpy(label, "Hello, world!\n");
  int i = (int)strlen(label);
  ulong c;
  for (c = ' ' + 1; c < 127; c++)
  {
    if (!(c & 0x1f))
      label[i++] = '\n';
    if (c == '@')
      label[i++] = '@';
    label[i++] = (char)c;
  }
  label[i++] = '\n';
  for (c = 0xA1; c < 0x600; c += 9)
  {
    if (!(++n & (0x1f)))
      label[i++] = '\n';
    i += fl_utf8encode((unsigned int)c, label + i);
  }
  label[i] = 0;

  // create the basic layout
  form = new Fl_Double_Window(550, 185);

  tile = new Fl_Tile(0, 0, 550, 185);

  Fl_Group *textgroup = new Fl_Group(0, 0, 550, 185);
  textgroup->box(FL_FLAT_BOX);
  textobj = new FontDisplay(FL_FRAME_BOX, 10, 10, 530, 170, label);
  textobj->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  textobj->color(9, 47);
  textgroup->resizable(textobj);
  textgroup->end();
  form->resizable(tile);
  form->end();
}

void create_font_selector()
{
  font_sel_win = new Fl_Double_Window(550, 200);
  //font_sel_win->clear_border();
  Fl_Group *fontgroup = new Fl_Group(0, 0, 550, 185);
  fontgroup->box(FL_FLAT_BOX);
  fontobj = new Fl_Hold_Browser(10, 5, 390, 170);
  fontobj->box(FL_FRAME_BOX);
  fontobj->color(53, 3);
  fontobj->callback(font_cb);
  sizeobj = new Fl_Hold_Browser(410, 5, 130, 170);
  sizeobj->box(FL_FRAME_BOX);
  sizeobj->color(53, 3);
  sizeobj->callback(size_cb);
  fontgroup->resizable(fontobj);
  fontgroup->end();

  tile->end();

  font_sel_win->resizable(tile);
  font_sel_win->end();

  int k = Fl::set_fonts("-*");
  sizes = new int *[k];
  numsizes = new int[k];
  for (int i = 0; i < k; i++)
  {
    int t;
    const char *name = Fl::get_font_name((Fl_Font)i, &t);
    char buffer[128];

    if (t)
    {
      char *p = buffer;
      if (t & FL_BOLD)
      {
        *p++ = '@';
        *p++ = 'b';
      }
      if (t & FL_ITALIC)
      {
        *p++ = '@';
        *p++ = 'i';
      }
      *p++ = '@';
      *p++ = '.';
      strcpy(p, name);
      name = buffer;
    }

    fontobj->add(name);
    std::cout << name;
    int *s;
    int n = Fl::get_font_sizes((Fl_Font)i, s);
    numsizes[i] = n;
    if (n)
    {
      std::cout << "; ";
      sizes[i] = new int[n];
      for (int j = 0; j < n; j++)
      {
        sizes[i][j] = s[j];
        std::cout << s[j] << ", ";
      }
    }
    std::cout << std::endl;
  }/*
  fontobj->value(1);
  font_cb(fontobj, 0);*/
}

int main(int argc, char **argv)
{/*
  Fl::scheme(NULL);
  Fl::args_to_utf8(argc, argv);
  Fl::args(argc, argv);
  Fl::get_system_colors();*/

  create_font_display();
  create_font_selector();

  form->show(argc,argv);
  font_sel_win->show(argc, argv);
  return Fl::run();
}
