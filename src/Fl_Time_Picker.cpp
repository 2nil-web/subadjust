// Fl_Time_Picker.cpp
#include "Fl_Time_Picker.H"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cstdio>
#include <cstdlib>
#include <cstring>

Fl_Time_Picker::Fl_Time_Picker() : Fl_Window(1, 1), use24_(false), show_ms_(false), ncols_(4)
{
  set_non_modal();
  border(0);
  box(FL_BORDER_BOX);
  color(FL_WHITE);
  cur_ = {12, 0, 0, 0, false};
  for (int i = 0; i < 5; i++)
    sc_[i] = 0;
  fm_.measure(FL_HELVETICA, FL_NORMAL_SIZE);
  memset(col_x_, 0, sizeof(col_x_));
  memset(col_w_, 0, sizeof(col_w_));
  layout();
  end();
}

// Retourne l'index de colonne pour AM/PM (-1 si absent).
int Fl_Time_Picker::ampm_col() const
{
  if (use24_)
    return -1;
  return show_ms_ ? 4 : 3;
}

// Retourne l'index de colonne pour les millisecondes (-1 si absent).
int Fl_Time_Picker::ms_col() const
{
  return show_ms_ ? 3 : -1;
}

int Fl_Time_Picker::col_count(int col) const
{
  if (col == 0)
    return use24_ ? 24 : 12;
  if (col == 1 || col == 2)
    return 60;
  if (col == ms_col())
    return 1000;
  if (col == ampm_col())
    return 2;
  return 60;
}

int Fl_Time_Picker::total_w() const
{
  // La largeur totale est le bord droit de la dernière colonne,
  // ce qui tient compte des espaces inter-colonnes (separateurs :. ).
  if (ncols_ <= 0)
    return fm_.cw_num * 4;
  return col_x_[ncols_ - 1] + col_w_[ncols_ - 1];
}

void Fl_Time_Picker::set_config(const TimeFontMetrics &fm, bool use24, bool show_ms, const int *cx, const int *cw, int ncols)
{
  fm_ = fm;
  use24_ = use24;
  show_ms_ = show_ms;
  ncols_ = ncols;
  for (int i = 0; i < ncols; i++)
  {
    col_x_[i] = cx[i];
    col_w_[i] = cw[i];
  }
  layout();
}

void Fl_Time_Picker::layout()
{
  header_h_ = fm_.ch;
  btn_h_ = fm_.ch;
  btn_w_ = (int)(fm_.cw_ampm * 1.4);
  footer_h_ = btn_h_ + fm_.pad * 2;
  int tw = total_w();
  if (tw < 1)
    tw = fm_.cw_num * 4;
  size(tw, header_h_ + ROWS_V * fm_.ch + footer_h_);
}

void Fl_Time_Picker::clamp_scroll(int col)
{
  int cnt = col_count(col);
  sc_[col] = ((sc_[col] % cnt) + cnt) % cnt;
}

void Fl_Time_Picker::update_cur_from_scroll()
{
  cur_.h = use24_ ? sc_[0] : sc_[0] + 1;
  cur_.m = sc_[1];
  cur_.s = sc_[2];
  cur_.ms = show_ms_ ? sc_[ms_col()] : 0;
  int ac = ampm_col();
  cur_.pm = (ac >= 0) ? (sc_[ac] == 1) : false;
}

void Fl_Time_Picker::accept_and_hide()
{
  Fl::grab(nullptr);
  if (on_accept)
    on_accept(cur_);
  hide();
}

void Fl_Time_Picker::cancel_and_hide()
{
  Fl::grab(nullptr);
  hide();
}

int Fl_Time_Picker::col_at(int mx) const
{
  // Cherche d'abord la colonne qui contient mx.
  for (int c = 0; c < ncols_; c++)
    if (mx >= col_x_[c] && mx < col_x_[c] + col_w_[c])
      return c;
  // Si mx est dans un separateur, retourne la colonne la plus proche.
  int best = 0, best_dist = std::abs(mx - (col_x_[0] + col_w_[0] / 2));
  for (int c = 1; c < ncols_; c++)
  {
    int dist = std::abs(mx - (col_x_[c] + col_w_[c] / 2));
    if (dist < best_dist)
    {
      best_dist = dist;
      best = c;
    }
  }
  return best;
}

void Fl_Time_Picker::item_label(int col, int idx, char *buf, int bufsz) const
{
  int ac = ampm_col();
  int mc = ms_col();
  if (col == ac)
    snprintf(buf, bufsz, "%s", idx == 0 ? "AM" : "PM");
  else if (col == mc)
    snprintf(buf, bufsz, "%03d", idx);
  else if (col == 0 && !use24_)
    snprintf(buf, bufsz, "%02d", idx + 1);
  else
    snprintf(buf, bufsz, "%02d", idx);
}

void Fl_Time_Picker::draw_col(int col, int cx, int cw)
{
  int cnt = col_count(col);
  int sel = sc_[col];
  int center = ROWS_V / 2;

  fl_push_clip(cx, clip_top(), cw, ROWS_V * fm_.ch);

  fl_color(0x0078d4ff);
  fl_rectf(cx, clip_top() + center * fm_.ch, cw, fm_.ch);

  fl_font(fm_.font, fm_.size);
  char buf[8];
  for (int row = 0; row < ROWS_V; row++)
  {
    int item_idx = ((sel - center + row) % cnt + cnt) % cnt;
    int iy = clip_top() + row * fm_.ch;
    fl_color(row == center ? FL_WHITE : FL_BLACK);
    item_label(col, item_idx, buf, sizeof(buf));
    fl_draw(buf, cx, iy, cw, fm_.ch, FL_ALIGN_CENTER);
  }

  fl_pop_clip();
}

void Fl_Time_Picker::draw()
{
  fl_draw_box(FL_BORDER_BOX, 0, 0, w(), h(), FL_WHITE);

  // En-têtes alignés sur les colonnes du widget parent.
  const char *hdrs24[] = {"H", " Min", " Sec", "ms", nullptr};
  const char *hdrs12[] = {"H", " Min", " Sec", "AM/PM", nullptr};
  const char *hdrs24ms[] = {"H", " Min", " Sec", "ms", "AM/PM"};
  const char *hdrs12ms[] = {"H", " Min", " Sec", "ms", "AM/PM"};
  const char **hdrs = use24_ ? (show_ms_ ? hdrs24ms : hdrs24) : (show_ms_ ? hdrs12ms : hdrs12);

  fl_font(fm_.font, fm_.size - 4);
  fl_color(0);
  for (int c = 0; c < ncols_; c++)
  {
    fl_push_clip(col_x_[c], 0, col_w_[c], header_h_);
    fl_draw(hdrs[c], col_x_[c], 0, col_w_[c], header_h_, FL_ALIGN_CENTER);
    fl_pop_clip();
  }

  // fl_color(fl_lighter(FL_GRAY));
  fl_color(FL_BLACK);
  fl_line(0, header_h_ - 1, w(), header_h_ - 1);

  // Separateurs verticaux entre colonnes (sur le bord gauche de chaque colonne sauf la 1ere).
  // fl_color(fl_lighter(FL_GRAY));
  fl_color(FL_BLACK);
  for (int c = 1; c < ncols_; c++)
    fl_line(col_x_[c], 0, col_x_[c], h() - footer_h_);

  for (int c = 0; c < ncols_; c++)
    draw_col(c, col_x_[c], col_w_[c]);

  int fy = h() - footer_h_;
  fl_color(FL_BLACK);
  // fl_color(fl_lighter(FL_GRAY));
  fl_line(0, fy, w(), fy);

  int bx = ok_x(), by = btn_y();
  fl_draw_box(FL_FLAT_BOX, bx, by, btn_w_, btn_h_, 0x0078d4ff);
  fl_font(fm_.font, fm_.size);
  fl_color(FL_WHITE);
  fl_draw("OK", bx, by, btn_w_, btn_h_, FL_ALIGN_CENTER);
}

int Fl_Time_Picker::handle(int ev)
{
  switch (ev)
  {
  case FL_PUSH: {
    int mx = Fl::event_x(), my = Fl::event_y();
    if (mx < 0 || mx >= w() || my < 0 || my >= h())
    {
      cancel_and_hide();
      return 1;
    }
    {
      int bx = ok_x(), by = btn_y();
      if (my >= by && my <= by + btn_h_ && mx >= bx && mx <= bx + btn_w_)
      {
        accept_and_hide();
        return 1;
      }
    }
    if (my >= clip_top() && my < clip_bottom())
    {
      int col = col_at(mx);
      int delta = (my - clip_top()) / fm_.ch - ROWS_V / 2;
      sc_[col] += delta;
      clamp_scroll(col);
      update_cur_from_scroll();
      redraw();
      return 1;
    }
    return 1;
  }
  case FL_MOUSEWHEEL:
    if (Fl::event_y() >= clip_top() && Fl::event_y() < clip_bottom())
    {
      int col = col_at(Fl::event_x());
      sc_[col] += Fl::event_dy();
      clamp_scroll(col);
      update_cur_from_scroll();
      redraw();
      return 1;
    }
    return 0;
  case FL_KEYDOWN:
    if (Fl::event_key() == FL_Escape)
    {
      cancel_and_hide();
      return 1;
    }
    if (Fl::event_key() == FL_Enter)
    {
      accept_and_hide();
      return 1;
    }
    break;
  }
  return Fl_Window::handle(ev);
}

void Fl_Time_Picker::show_at(int screen_x, int screen_y, Time t)
{
  sc_[0] = use24_ ? t.h : t.h - 1;
  sc_[1] = t.m;
  sc_[2] = t.s;
  int mc = ms_col();
  int ac = ampm_col();
  if (mc >= 0)
    sc_[mc] = t.ms;
  if (ac >= 0)
    sc_[ac] = t.pm ? 1 : 0;
  update_cur_from_scroll();
  position(screen_x, screen_y);
  show();
  Fl::grab(*this);
  redraw();
}
