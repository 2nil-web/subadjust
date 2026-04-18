// Fl_Time_Input.cpp
#include "Fl_Time_Input.H"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cctype>
#include <cstdio>
#include <cstring>

#include "log.h"

// ----------------------------------------------------------------
// Constructeur / destructeur
// ----------------------------------------------------------------

Fl_Time_Input::Fl_Time_Input(int X, int Y, int W, int H, const char *lbl, bool use24, bool show_ms)
    : Fl_Group(X, Y, W, H, lbl), h_(use24 ? 0 : 12), m_(0), s_(0), ms_(0), pm_(false), use24_(use24), show_ms_(show_ms), active_seg_(SEG_NONE), digit_count_(0), digit_accum_(0), picker_(nullptr), user_cb_(nullptr), user_data_(nullptr)
{
  end();
  box(FL_NO_BOX);
}

Fl_Time_Input::~Fl_Time_Input()
{
  delete picker_;
}

// ----------------------------------------------------------------
// Configuration dynamique
// ----------------------------------------------------------------

void Fl_Time_Input::set_mode(bool use24, bool show_ms)
{
  use24_ = use24;
  show_ms_ = show_ms;
  // Recadre l'heure si on passe de 12h à 24h ou inversement.
  clamp_values();
  active_seg_ = SEG_NONE;
  digit_count_ = 0;
  digit_accum_ = 0;
  delete picker_;
  picker_ = nullptr;
  fit();
  redraw();
}

// ----------------------------------------------------------------
// Valeur
// ----------------------------------------------------------------

void Fl_Time_Input::clamp_values()
{
  if (use24_)
  {
    if (h_ < 0)
      h_ = 0;
    if (h_ > 23)
      h_ = 23;
  }
  else
  {
    if (h_ < 1)
      h_ = 1;
    if (h_ > 12)
      h_ = 12;
  }
  if (m_ < 0)
    m_ = 0;
  if (m_ > 59)
    m_ = 59;
  if (s_ < 0)
    s_ = 0;
  if (s_ > 59)
    s_ = 59;
  if (ms_ < 0)
    ms_ = 0;
  if (ms_ > 999)
    ms_ = 999;
}

void Fl_Time_Input::set_time(int hour, int min, int sec, int ms, bool pm)
{
  h_ = hour;
  m_ = min;
  s_ = sec;
  ms_ = ms;
  pm_ = pm;
  clamp_values();
  redraw();
}

void Fl_Time_Input::set_time_ms(int millisec)
{
  int ms, sec, min, kp_sec, kp_min, hour;
  kp_sec = millisec / 1000;
  ms = millisec % 1000; // ms à garder

  kp_min = kp_sec / 60;
  sec = kp_sec % 60; // sec à garder

  hour = kp_min / 60; // hour à garder
  min = kp_min % 60;  // min à garder

  h_ = hour;
  m_ = min;
  s_ = sec;
  ms_ = ms;

  clamp_values();
  redraw();
}

// Set time with a string of the form HH:MM:SS./,SSS AM/PM
// string can be empty
void Fl_Time_Input::set_time_str(std::string str)
{
  if (str.size() >= 2)
    h_ = std::stoi(str.substr(0, 2));
  else
    h_ = 0;

  if (str.size() >= 5)
    m_ = std::stoi(str.substr(3, 2));
  else
    m_ = 0;

  if (str.size() >= 8)
    s_ = std::stoi(str.substr(6, 2));
  else
    s_ = 0;

  if (str.size() >= 12)
    ms_ = std::stoi(str.substr(9, 3));
  else
    ms_ = 0;

  if (str.size() >= 14)
  {
    if (str[13] == 'A' || str[13] == 'a')
      pm_ = false;
    if (str[13] == 'P' || str[13] == 'p')
      pm_ = true;
  }

  clamp_values();
  redraw();
}

int Fl_Time_Input::get_time_ms()
{
  int ms = h_ * 3600000;
  ms += m_ * 60000;
  ms += s_ * 1000;
  ms += ms_;

  return ms;
}

void Fl_Time_Input::get_time(int &h, int &m, int &s, int &ms)
{
  h = h_;
  m = m_;
  s = s_;
  ms = ms_;
}

// Convert an int to a string padding on the left with 2 or 3 zeros
std::string pad2(unsigned int d, bool two = true)
{
  size_t np = (two ? 2 : 3);

  std::string s = std::to_string(d);
  for (size_t i = s.size(); i < np; i++)
    s = '0' + s;
  return s;
}

std::string Fl_Time_Input::get_time_str()
{
  std::string s = pad2(hour24());
  s += ":" + pad2(m_);
  s += ":" + pad2(s_);
  s += "," + pad2(ms_, false);

  if (!use24_)
    s += std::string(" ") + (pm_ ? "PM" : "AM");

  logD("get_time_str: ", s);
  return s;
}

int Fl_Time_Input::hour24() const
{
  if (use24_)
    return h_;
  if (pm_)
    return h_ == 12 ? 12 : h_ + 12;
  return h_ == 12 ? 0 : h_;
}

// ----------------------------------------------------------------
// Segments : nombre, max, navigation
// ----------------------------------------------------------------

int Fl_Time_Input::nseg() const
{
  int n = 3;
  if (show_ms_)
    n++;
  if (!use24_)
    n++;
  return n;
}

int Fl_Time_Input::seg_max(TimeSegment s) const
{
  switch (s)
  {
  case SEG_HOUR:
    return use24_ ? 23 : 12;
  case SEG_MIN:
    return 59;
  case SEG_SEC:
    return 59;
  case SEG_MS:
    return 999;
  default:
    return 1;
  }
}

// Retourne le prochain segment valide dans la configuration courante.
TimeSegment Fl_Time_Input::next_seg(TimeSegment s) const
{
  int n = (int)s;
  for (int i = 1; i <= 5; i++)
  {
    TimeSegment c = (TimeSegment)((n + i) % 5);
    if (c == SEG_MS && !show_ms_)
      continue;
    if (c == SEG_AMPM && use24_)
      continue;
    if (c == SEG_NONE)
      continue;
    return c;
  }
  return SEG_HOUR;
}

TimeSegment Fl_Time_Input::prev_seg(TimeSegment s) const
{
  int n = (int)s;
  for (int i = 1; i <= 5; i++)
  {
    int idx = ((n - i) % 5 + 5) % 5;
    TimeSegment c = (TimeSegment)idx;
    if (c == SEG_MS && !show_ms_)
      continue;
    if (c == SEG_AMPM && use24_)
      continue;
    if (c == SEG_NONE)
      continue;
    return c;
  }
  return SEG_SEC;
}

// ----------------------------------------------------------------
// Copier / coller
// ----------------------------------------------------------------

void Fl_Time_Input::format_clipboard(char *buf, int bufsz) const
{
  if (show_ms_)
    snprintf(buf, bufsz, "%02d:%02d:%02d,%03d%s", hour24(), m_, s_, ms_, "");
  else
    snprintf(buf, bufsz, "%02d:%02d:%02d", hour24(), m_, s_);
}

bool Fl_Time_Input::parse_clipboard(const char *str)
{
  int hh = 0, mm = 0, ss = 0, mss = 0;
#ifdef _WIN32
  int n = sscanf_s(str, "%d:%d:%d,%d", &hh, &mm, &ss, &mss);
#else
  int n = sscanf(str, "%d:%d:%d,%d", &hh, &mm, &ss, &mss);
#endif
  if (n < 3)
    return false;
  if (use24_)
  {
    if (hh < 0 || hh > 23)
      return false;
    h_ = hh;
    pm_ = false;
  }
  else
  {
    if (hh < 0 || hh > 23)
      return false;
    pm_ = hh >= 12;
    h_ = hh % 12;
    if (h_ == 0)
      h_ = 12;
  }
  if (mm < 0 || mm > 59)
    return false;
  if (ss < 0 || ss > 59)
    return false;
  m_ = mm;
  s_ = ss;
  ms_ = (n >= 4) ? mss : 0;
  clamp_values();
  return true;
}

void Fl_Time_Input::copy_value()
{
  char buf[32];
  format_clipboard(buf, sizeof(buf));
  Fl::copy(buf, (int)strlen(buf), 1);
}

void Fl_Time_Input::paste_value()
{
  Fl::paste(*this, 1);
}

// ----------------------------------------------------------------
// Police et géométrie
// ----------------------------------------------------------------

void Fl_Time_Input::measure_font()
{
  fm_.measure(labelfont(), labelsize());
}

void Fl_Time_Input::preferred_size(int &pw, int &ph) const
{
  TimeFontMetrics m;
  m.measure(labelfont(), labelsize());

  int clock_w = m.ch;
  int spin_w = m.ch / 2 + 2;

  pw = m.pad + m.cw_num + m.sep_w // HH:
       + m.cw_num + m.sep_w       // MM:
       + m.cw_num;                // SS
  if (show_ms_)
    pw += m.dot_w + m.cw_ms; // .mmm
  if (!use24_)
    pw += m.pad + m.cw_ampm; // AM/PM
  else
    pw += m.pad;
  pw += spin_w + clock_w;

  ph = m.ch;
  logD("pref sz - pw: ", pw, ", ph: ", ph);
}

void Fl_Time_Input::fit()
{
  int pw, ph;
  preferred_size(pw, ph);
  Fl_Group::resize(x(), y(), pw, ph);
}

void Fl_Time_Input::resize(int X, int Y, int W, int H)
{
  int pw, ph;
  preferred_size(pw, ph);
  (void)W;
  (void)H;
  Fl_Group::resize(X, Y, pw, ph);
}

void Fl_Time_Input::compute_rects()
{
  measure_font();

  int rh = fm_.ch;
  int ry = y() + (h() - rh) / 2;
  int clock_w = rh;
  int spin_w = rh / 2 + 2;

  r_.cw = clock_w;
  r_.ch = rh;
  r_.cx = x() + w() - clock_w;
  r_.cy = ry;
  r_.sw = spin_w;
  r_.sh = rh;
  r_.sx = r_.cx - spin_w;
  r_.sy = ry;
  r_.fx = x();
  r_.fy = ry;
  r_.fw = r_.sx - x();
  r_.fh = rh;

  int pad = fm_.pad;
  int px = r_.fx + pad;
  int si = 0;

  r_.seg_x[SEG_HOUR] = px;
  r_.seg_w[SEG_HOUR] = fm_.cw_num;
  px += fm_.cw_num + fm_.sep_w;
  r_.seg_x[SEG_MIN] = px;
  r_.seg_w[SEG_MIN] = fm_.cw_num;
  px += fm_.cw_num + fm_.sep_w;
  r_.seg_x[SEG_SEC] = px;
  r_.seg_w[SEG_SEC] = fm_.cw_num;
  px += fm_.cw_num;
  si = 3;

  if (show_ms_)
  {
    px += fm_.dot_w;
    r_.seg_x[SEG_MS] = px;
    r_.seg_w[SEG_MS] = fm_.cw_ms;
    px += fm_.cw_ms;
    si++;
  }
  else
  {
    r_.seg_x[SEG_MS] = -1;
    r_.seg_w[SEG_MS] = 0;
  }

  px += pad;
  if (!use24_)
  {
    r_.seg_x[SEG_AMPM] = px;
    r_.seg_w[SEG_AMPM] = fm_.cw_ampm;
    si++;
  }
  else
  {
    r_.seg_x[SEG_AMPM] = -1;
    r_.seg_w[SEG_AMPM] = 0;
  }

  r_.nseg = si;
}

// ----------------------------------------------------------------
// Dessin
// ----------------------------------------------------------------

void Fl_Time_Input::draw_segment(const char *txt, int sx, int sy, int sw, int sh, bool active)
{
  if (active)
  {
    fl_color(0x0078d4ff);
    fl_rectf(sx, sy + 1, sw, sh - 2);
    fl_color(FL_WHITE);
  }
  else
  {
    fl_color(FL_FOREGROUND_COLOR);
  }
  fl_font(fm_.font, fm_.size);
  fl_draw(txt, sx, sy, sw, sh, FL_ALIGN_CENTER);
}

void Fl_Time_Input::draw_spin_button(int bx, int by, int bw, int bh)
{
  fl_draw_box(FL_BORDER_BOX, bx, by, bw, bh, FL_BACKGROUND_COLOR);
  int mx = bx + bw / 2;
  int aw = bw / 3, am = bh / 5;
  fl_color(0);
  fl_polygon(mx - aw, by + bh / 2 - 1, mx + aw, by + bh / 2 - 1, mx, by + am);
  fl_polygon(mx - aw, by + bh / 2 + 1, mx + aw, by + bh / 2 + 1, mx, by + bh - am);
}

void Fl_Time_Input::draw_clock_button(int bx, int by, int bw, int bh)
{
  fl_draw_box(FL_BORDER_BOX, bx, by, bw, bh, FL_BACKGROUND_COLOR);
  int cx = bx + bw / 2, cy = by + bh / 2, cr = bh / 3;
  fl_color(0);
  fl_circle(cx, cy, cr);
  fl_line(cx, cy, cx, cy - cr + cr / 4 + 1);
  fl_line(cx, cy, cx + cr - cr / 4 - 3, cy);
}

void Fl_Time_Input::draw()
{
  compute_rects();
  fl_draw_box(FL_BORDER_BOX, r_.fx, r_.fy, r_.fw, r_.fh, FL_WHITE);

  fl_font(fm_.font, fm_.size);
  fl_color(FL_DARK3);

  // Séparateur HH:MM
  fl_draw(":", r_.seg_x[SEG_HOUR] + r_.seg_w[SEG_HOUR], r_.fy, fm_.sep_w, r_.fh, FL_ALIGN_CENTER);
  // Séparateur MM:SS
  fl_draw(":", r_.seg_x[SEG_MIN] + r_.seg_w[SEG_MIN], r_.fy, fm_.sep_w, r_.fh, FL_ALIGN_CENTER);
  // Séparateur SS.mmm
  if (show_ms_)
    fl_draw(",", r_.seg_x[SEG_SEC] + r_.seg_w[SEG_SEC], r_.fy, fm_.dot_w, r_.fh, FL_ALIGN_CENTER);

  char buf[8];

  snprintf(buf, sizeof(buf), "%02d", h_);
  draw_segment(buf, r_.seg_x[SEG_HOUR], r_.fy, r_.seg_w[SEG_HOUR], r_.fh, active_seg_ == SEG_HOUR);

  snprintf(buf, sizeof(buf), "%02d", m_);
  draw_segment(buf, r_.seg_x[SEG_MIN], r_.fy, r_.seg_w[SEG_MIN], r_.fh, active_seg_ == SEG_MIN);

  snprintf(buf, sizeof(buf), "%02d", s_);
  draw_segment(buf, r_.seg_x[SEG_SEC], r_.fy, r_.seg_w[SEG_SEC], r_.fh, active_seg_ == SEG_SEC);

  if (show_ms_)
  {
    snprintf(buf, sizeof(buf), "%03d", ms_);
    draw_segment(buf, r_.seg_x[SEG_MS], r_.fy, r_.seg_w[SEG_MS], r_.fh, active_seg_ == SEG_MS);
  }

  if (!use24_)
    draw_segment(pm_ ? "PM" : "AM", r_.seg_x[SEG_AMPM], r_.fy, r_.seg_w[SEG_AMPM], r_.fh, active_seg_ == SEG_AMPM);

  draw_spin_button(r_.sx, r_.sy, r_.sw, r_.sh);
  draw_clock_button(r_.cx, r_.cy, r_.cw, r_.ch);
  draw_label();
}

// ----------------------------------------------------------------
// Navigation / saisie
// ----------------------------------------------------------------

void Fl_Time_Input::activate_segment(TimeSegment s)
{
  flush_digit();
  active_seg_ = s;
  digit_count_ = 0;
  digit_accum_ = 0;
  redraw();
}

void Fl_Time_Input::increment(int delta)
{
  flush_digit();
  switch (active_seg_)
  {
  case SEG_HOUR:
    if (use24_)
    {
      h_ = (h_ + delta + 24) % 24;
    }
    else
    {
      h_ += delta;
      if (h_ > 12)
        h_ = 1;
      if (h_ < 1)
        h_ = 12;
    }
    break;
  case SEG_MIN:
    m_ = (m_ + delta + 60) % 60;
    break;
  case SEG_SEC:
    s_ = (s_ + delta + 60) % 60;
    break;
  case SEG_MS:
    ms_ = (ms_ + delta + 1000) % 1000;
    break;
  case SEG_AMPM:
    pm_ = !pm_;
    break;
  default:
    break;
  }
  redraw();
  fire_callback();
}

// Accumule un chiffre dans le segment actif et avance si complet.
void Fl_Time_Input::commit_digit(int digit)
{
  if (active_seg_ == SEG_AMPM || active_seg_ == SEG_NONE)
    return;

  int max_digits = (active_seg_ == SEG_MS) ? 3 : 2;
  digit_accum_ = digit_accum_ * 10 + digit;
  digit_count_++;

  // Applique la valeur partielle immédiatement (défilement gauche→droite).
  switch (active_seg_)
  {
  case SEG_HOUR:
    if (use24_)
      h_ = digit_accum_ % 24;
    else
      h_ = (digit_accum_ == 0) ? 12 : digit_accum_ % 13;
    break;
  case SEG_MIN:
    m_ = digit_accum_ % 60;
    break;
  case SEG_SEC:
    s_ = digit_accum_ % 60;
    break;
  case SEG_MS:
    ms_ = digit_accum_ % 1000;
    break;
  default:
    break;
  }

  redraw();
  fire_callback();

  // Avance automatiquement au segment suivant quand c'est complet.
  bool full = (digit_count_ >= max_digits);
  if (!full && active_seg_ == SEG_HOUR && !use24_ && digit_accum_ > 1)
    full = true; // 2..9 en 12h : un seul chiffre suffit
  if (!full && active_seg_ == SEG_HOUR && use24_ && digit_accum_ > 2)
    full = true; // 3..9 en 24h

  if (full)
  {
    digit_count_ = 0;
    digit_accum_ = 0;
    active_seg_ = next_seg(active_seg_);
    redraw();
  }
}

// Valide et réinitialise l'accumulation en cours sans changer de segment.
void Fl_Time_Input::flush_digit()
{
  digit_count_ = 0;
  digit_accum_ = 0;
}

// ----------------------------------------------------------------
// Picker
// ----------------------------------------------------------------

void Fl_Time_Input::open_picker()
{
  measure_font();

  if (!picker_)
  {
    picker_ = new Fl_Time_Picker();
    picker_->on_accept = [this](Fl_Time_Picker::Time t) {
      h_ = t.h;
      m_ = t.m;
      s_ = t.s;
      ms_ = t.ms;
      pm_ = t.pm;
      clamp_values();
      redraw();
      fire_callback();
    };
  }

  // Transmet la configuration et les positions des colonnes du widget.
  int cx[5], cw[5];
  cx[0] = r_.seg_x[SEG_HOUR] - x();
  cw[0] = r_.seg_w[SEG_HOUR];
  cx[1] = r_.seg_x[SEG_MIN] - x();
  cw[1] = r_.seg_w[SEG_MIN];
  cx[2] = r_.seg_x[SEG_SEC] - x();
  cw[2] = r_.seg_w[SEG_SEC];
  int nc = 3;
  if (show_ms_)
  {
    cx[nc] = r_.seg_x[SEG_MS] - x();
    cw[nc] = r_.seg_w[SEG_MS];
    nc++;
  }
  if (!use24_)
  {
    cx[nc] = r_.seg_x[SEG_AMPM] - x();
    cw[nc] = r_.seg_w[SEG_AMPM];
    nc++;
  }

  picker_->set_config(fm_, use24_, show_ms_, cx, cw, nc);

  int screen_x = window()->x() + x();
  int screen_y = window()->y() + y() + h();
  if (screen_y + picker_->h() > Fl::h())
    screen_y = window()->y() + y() - picker_->h();

  picker_->show_at(screen_x, screen_y, {h_, m_, s_, ms_, pm_});
}

// ----------------------------------------------------------------
// Callback
// ----------------------------------------------------------------

void Fl_Time_Input::callback(Fl_Callback *cb, void *data)
{
  user_cb_ = cb;
  user_data_ = data;
}

void Fl_Time_Input::fire_callback()
{
  if (user_cb_)
    user_cb_(this, user_data_);
}

// ----------------------------------------------------------------
// Événements
// ----------------------------------------------------------------

int Fl_Time_Input::handle(int ev)
{
  compute_rects();

  switch (ev)
  {
  case FL_PUSH: {
    take_focus();
    int mx = Fl::event_x(), my = Fl::event_y();

    if (mx >= r_.cx && mx <= r_.cx + r_.cw && my >= r_.cy && my <= r_.cy + r_.ch)
    {
      open_picker();
      return 1;
    }
    if (mx >= r_.sx && mx <= r_.sx + r_.sw && my >= r_.sy && my <= r_.sy + r_.sh)
    {
      if (active_seg_ == SEG_NONE)
        active_seg_ = SEG_HOUR;
      increment(my < r_.sy + r_.sh / 2 ? 1 : -1);
      return 1;
    }
    // Clic sur un segment — seulement ceux présents dans la config.
    TimeSegment order[] = {SEG_HOUR, SEG_MIN, SEG_SEC, SEG_MS, SEG_AMPM};
    for (int i = 0; i < 5; i++)
    {
      TimeSegment s = order[i];
      if (r_.seg_x[s] < 0)
        continue;
      if (mx >= r_.seg_x[s] && mx <= r_.seg_x[s] + r_.seg_w[s])
      {
        activate_segment(s);
        return 1;
      }
    }
    return 1;
  }

  case FL_MOUSEWHEEL:
    if (active_seg_ != SEG_NONE)
    {
      increment(-Fl::event_dy());
      return 1;
    }
    return 0;

  case FL_KEYDOWN: {
    int key = Fl::event_key();

    if (key == FL_Up)
    {
      if (active_seg_ == SEG_NONE)
        active_seg_ = SEG_HOUR;
      increment(1);
      return 1;
    }
    if (key == FL_Down)
    {
      if (active_seg_ == SEG_NONE)
        active_seg_ = SEG_HOUR;
      increment(-1);
      return 1;
    }
    if (key == FL_Tab && !Fl::event_shift())
    {
      flush_digit();
      active_seg_ = (active_seg_ == SEG_NONE) ? SEG_HOUR : next_seg(active_seg_);
      redraw();
      return 1;
    }
    if (key == FL_Tab && Fl::event_shift())
    {
      flush_digit();
      active_seg_ = (active_seg_ == SEG_NONE) ? SEG_SEC : prev_seg(active_seg_);
      redraw();
      return 1;
    }
    if (key == FL_Right)
    {
      flush_digit();
      active_seg_ = next_seg(active_seg_);
      redraw();
      return 1;
    }
    if (key == FL_Left)
    {
      flush_digit();
      active_seg_ = prev_seg(active_seg_);
      redraw();
      return 1;
    }

    // Saisie numerique : clavier principal et pave numerique.
    {
      int digit = -1;
      if (key >= '0' && key <= '9')
        digit = key - '0';
      else if (key >= FL_KP && key <= FL_KP + '9')
        digit = key - FL_KP - '0';
      if (digit >= 0 && active_seg_ != SEG_AMPM && active_seg_ != SEG_NONE)
      {
        commit_digit(digit);
        return 1;
      }
    }

    // Bascule AM/PM.
    if (!use24_)
    {
      if (key == 'a' || key == 'A')
      {
        pm_ = false;
        redraw();
        fire_callback();
        return 1;
      }
      if (key == 'p' || key == 'P')
      {
        pm_ = true;
        redraw();
        fire_callback();
        return 1;
      }
    }

    // Copier/coller : Fl::event_state(FL_CTRL) est le test fiable.
    if (Fl::event_state(FL_CTRL) && (key == 'c' || key == 'C'))
    {
      copy_value();
      return 1;
    }
    if (Fl::event_state(FL_CTRL) && (key == 'v' || key == 'V'))
    {
      paste_value();
      return 1;
    }

    break;
  }

  // Réception du texte collé (réponse asynchrone à Fl::paste).
  case FL_PASTE: {
    const char *txt = Fl::event_text();
    if (txt && parse_clipboard(txt))
    {
      redraw();
      fire_callback();
    }
    return 1;
  }

  case FL_FOCUS:
    if (active_seg_ == SEG_NONE)
      active_seg_ = SEG_HOUR;
    redraw();
    return 1;

  case FL_UNFOCUS:
    flush_digit();
    active_seg_ = SEG_NONE;
    redraw();
    return 1;
  }

  return Fl_Group::handle(ev);
}
