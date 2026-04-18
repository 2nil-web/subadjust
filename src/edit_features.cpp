
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <thread>

#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/fl_ask.H>
#include <FL/platform.H>

#include "edit_features.h"
#include "file_features.h"
#include "log.h"
#include "subadjust_ui.h"
#include "subs.h"
#include "themes.h"
#include "utils.h"

// Expect an int that when positive, is a line index
// When negative, take its absolute value as a percentage of the line count
void to_line(int d)
{
  logT("to_line - d: ", d);
  if (txt_buf.text() == nullptr || txt_buf.count_lines(0, -1) < 1)
    return;

  file_content->scroll(1, 0);
  int line, nl = file_content->count_lines(0, -1, true);

  if (d >= 0)
  {
    if (d > nl)
    {
      line = 0;
      d = 0;
    }
    else
      line = d - 1;
  }
  else
  {
    d = -d;
    if (d >= 100)
      line = nl;
    else
      line = (int)std::round((double)(d * nl) / 100.0) - 1;
  }

  if (goto_line->maximum() != nl)
    goto_line->maximum(nl);

  logT("to_line: ", line);
  file_content->scroll(line, 0);
  if (goto_line->value() != line)
    goto_line->value(line + 1);

  int pos = txt_buf.skip_lines(0, line);
  pos = file_content->line_start(pos);
  logT("to_line1 - pos: ", pos, ", line: ", line, ", nl: ", nl);
  int pos1 = pos;

  std::string nsub, stime;
  if (!txt_buf.search_forward(pos1, " -->", &pos1))
  {
    txt_buf.search_backward(pos1, " -->", &pos1);
  }
  stime = txt_buf.text_range(pos1 - 12, pos1);
  int pos2;
  txt_buf.search_backward(pos1 - 13, "\n", &pos2);
  txt_buf.search_backward(pos2 - 1, "\n", &pos1);
  nsub = txt_buf.text_range(pos1 + 1, pos2);
  double ins = std::stod(nsub);
  if (ins < 1)
    ins = 1;
  goto_sub->value(ins);
  time_to_go->set_time_str(stime);
  file_content->insert_position(pos);
  logT("to_line2 - pos: [", pos, "], line: [", line, "], sub: [", ins, "], stime: [", stime, "]");
}

// Expect an int that when positive, is a subtitle index
// When negative, take its absolute value as a percentage of the subtitle count
int to_sub(int d)
{
  file_content->scroll(1, 0);
  csub.parse(txt_buf.text());
  int ins = d;
  int ns = (int)csub.vec().size();
  if (goto_sub->maximum() > (int)ns)
    goto_sub->maximum((int)ns);
  logT("to_sub1 - d: ", d, ", ins: ", ins);

  if (d >= 0)
  {
    if (d < 1)
    {
      ins = ns;
      d = ns;
    }
    else if (d > ns)
    {
      ins = 1;
      d = 1;
    }
  }
  else
  {
    d = -d;
    ins = (int)((double)(d * ns) / 100);
  }

  int nl = 1, pos = 1;
  std::string needle;
  int found;

  if (ins < 2)
    file_content->scroll(1, 0);
  else
  {
    std::string needle = "\n\n" + std::to_string(ins) + '\n';
    found = txt_buf.search_forward(0, needle.c_str(), &pos);
    if (found)
    {
      nl = 3 + file_content->count_lines(1, pos, true);
      file_content->scroll(nl, nl);
    }
  }

  if (ins < 1)
    ins = 1;
  logT("to_sub2 - ins: ", ins);
  if (goto_sub->value() != ins)
  {
    goto_sub->value(ins);
  }

  goto_line->value(nl);
  int pos2;
  if (d > 1)
  {
    txt_buf.search_forward(pos + 1, "\n", &pos);
    txt_buf.search_forward(pos + 1, "\n", &pos);
  }
  txt_buf.search_forward(pos + 1, " -->", &pos2);
  logT("to_sub3  - pos: ", pos, ", pos2: ", pos2);
  std::string stime = txt_buf.text_range(pos2 - 12, pos2);
  time_to_go->set_time_str(stime);
  logT("to_sub4  - text from pos:[", txt_buf.text_range(pos + 1, pos2), ']');
  pos = file_content->line_start(pos);
  file_content->insert_position(pos);
  logT("to_sub5  - pos: [", pos, "], line: [", nl, "], sub: [", ins, "], stime: [", stime, "]");

  return ins;
}

// Expect an int that when positive, is a time in milliseconds
// When negative, take its absolute value as a percentage of the subtitle duration
void to_time(int d)
{
  file_content->scroll(1, 0);
  csub.parse(txt_buf.text());

  if (d < 0)
  {
    d = -d * csub.vec()[csub.vec().size() - 1].disappearance / 100;
    logT("to_time1 - last ts: ", csub.vec()[csub.vec().size() - 1].disappearance, ", half ts: ", csub.vec()[csub.vec().size() - 1].disappearance / 2, ", d: ", d);
  }

  sSub ssub;
  int line = csub.line_by_timestamp(d, ssub);
  logT("to_time2(line_by_timestamp) - line: [", line, "], appearance: ", ms_to_str(ssub.appearance), ", disappearance: ", ms_to_str(ssub.disappearance));
  file_content->scroll(line, line);
  logT("to_time3 - line1: [", line, "]");
  int maxl = file_content->count_lines(0, -1, true);
  if (line < maxl)
    goto_line->value(line + 1);
  else
    goto_line->value(line);
  logT("to_time4 - line2: [", line, "]");
  int ins = ssub.index + 1;
  goto_sub->value(ins);
  time_to_go->set_time_ms(d);
  int pos = txt_buf.skip_lines(0, line);
  logT("to_time5 - line3: [", line, "]");

  file_content->insert_position(pos);
  std::string stime = ms_to_str(d);
  logT("to_time7 - pos: [", pos, "], line: [", line, "], sub: [", ins, "], stime: [", stime, "]", ", appearance: ", ms_to_str(ssub.appearance), ", disappearance: ", ms_to_str(ssub.disappearance));
}

bool is_valid_regex(std::regex &re)
{
  std::regex_constants::syntax_option_type syn = {};
  if (!case_sensitive_find->value())
    syn = std::regex_constants::icase;

  std::string sre = str_find->value();

  try
  {
    re = std::regex(sre, syn);
  }
  catch (const std::regex_error &e)
  {
    // fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
    std::string msg = e.what();
    replace_string_in_place(msg, ":", ":\n");
    fl_alert("%s", msg.c_str());
    return false;
  }

  return true;
}

bool find_pos1(std::string s, size_t &pos)
{
  std::regex pattern;
  if (is_valid_regex(pattern))
  {
    auto begin = std::sregex_iterator{s.begin(), s.end(), pattern};
    auto end = std::sregex_iterator();

    if (begin != end)
    {
      pos = begin->position();
      return true;
    }
  }

  return false;
}

void store_new_find(bool repl)
{
  std::string it = str_find->value();
  if (str_find->menubutton()->find_item(it.c_str()) == nullptr)
    str_find->add(it.c_str());

  if (repl)
  {
    it = str_replace->value();
    if (str_replace->menubutton()->find_item(it.c_str()) == nullptr)
      str_replace->add(it.c_str());
  }
}

Fl_Color init_col, blink_col;
int blink_loop;
void do_blink_regex(void *)
{
  if (blink_loop < 5)
  {
    Fl_Color actual_color = str_find->textcolor();
    if (actual_color == blink_col)
      str_find->textcolor(init_col);
    else
      str_find->textcolor(blink_col);
    Fl::repeat_timeout(0.3, do_blink_regex);
  }
  else
  {
    Fl::remove_timeout(do_blink_regex);
    str_find->textcolor(init_col);
  }

  str_find->redraw();
  main_window->redraw();
  blink_loop++;
}

void blink_regex(Fl_Color col)
{
  blink_loop = 0;
  init_col = str_find->textcolor();
  blink_col = col;
  Fl::add_timeout(0.3, do_blink_regex);
}

void regex_found(int new_pos, bool repl = true)
{
  // logT("new_pos: ", new_pos);
  blink_regex(FL_GREEN);
  store_new_find(repl);
  file_content->insert_position(new_pos);
  file_content->show_insert_position();
  Fl::focus(file_content);
  file_content->redraw();
}

void re_find(Fl_Widget *, void *)
{
  logT("file_content->insert_position(): ", file_content->insert_position());
  logT("str_find.value(): ", str_find->value());

  file_content->show_insert_position();

  int actual_pos = file_content->insert_position() + 1;
  size_t find_pos;
  if (find_pos1(txt_buf.text_range(actual_pos, txt_buf.length()), find_pos))
    regex_found(actual_pos + (int)find_pos, false);
  else
    blink_regex(FL_RED);
}

void re_replace(bool all)
{
  int actual_pos = file_content->insert_position();
  std::string s = txt_buf.text_range(actual_pos, txt_buf.length());
  std::regex rgx;

  if (is_valid_regex(rgx))
  {
    std::string res;

    auto begin = std::sregex_iterator{s.begin(), s.end(), rgx};

    if (all)
      res = std::regex_replace(s, rgx, str_replace->value());
    else
      res = std::regex_replace(s, rgx, str_replace->value(), std::regex_constants::format_first_only);

    // logT("re_replace: ", res, std::endl, res.size(), " chars length");
    if (s == res)
      blink_regex(FL_RED);
    else
    {
      txt_buf.replace(actual_pos, txt_buf.length(), res.c_str(), (int)res.size());
      regex_found(actual_pos + (int)begin->position());
    }
  }
}

void re_replace_next(Fl_Widget *, void *)
{
  re_replace(false);
}

void re_replace_all(Fl_Widget *, void *)
{
  re_replace();
}

void reparse(Fl_Widget *, void *)
{
  csub.parse(txt_buf.text());
  int ns = (int)csub.vec().size();
  if (goto_sub->maximum() > (int)ns)
    goto_sub->maximum((int)ns);
  txt_buf.replace(0, (int)csub.str().size(), csub.c_str());

  csub.parse(txt_buf.text());
  time_start->set_time_ms(csub.vec()[0].appearance);
  time_end->set_time_ms(csub.vec().back().appearance);
  offset_start->value(0);
  offset_stop->value(0);
  dur_coeff->value(1);
}

void delete_sub(int nsub)
{
  int real_sub = to_sub(nsub);
  int pos = file_content->insert_position();

  logT("delete_sub - real_sub: ", real_sub, ", pos: ", pos, ", goto_sub->maximum(): ", goto_sub->maximum(), ", txt_buf.length(): ", txt_buf.length());

  if (real_sub < goto_sub->maximum())
  {
    int pos2;
    if (txt_buf.search_forward(pos, "\n\n", &pos2))
    {
      txt_buf.remove(pos, pos2 + 2);
      if (real_sub > 1)
        real_sub--;
      reparse();

      to_sub(real_sub);
    }
  }
  else
  {
    txt_buf.remove(pos - 2, txt_buf.length());
    reparse();
    to_sub(-100);
  }

  Fl::focus(file_content);
  file_content->redraw();
}

void delete_first_sub(Fl_Widget *, void *)
{
  delete_sub(1);
}

void delete_last_sub(Fl_Widget *, void *)
{
  delete_sub(-100);
}
