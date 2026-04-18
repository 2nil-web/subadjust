
#include <iostream>
#include <sstream>

#include "log.h"
#include "subs.h"

size_t cSub::line_number()
{
  return nlines;
}

std::string cSub::str()
{
  return sub_str;
}

const char *cSub::c_str()
{
  return sub_str.c_str();
}

std::vector<sSub> cSub::vec()
{
  return sub_vec;
}

std::vector<sSub> cSub::to_vec()
{
  if (sub_str.empty())
    return {};

  std::vector<sSub> _vec;
  std::smatch sub_match;
  sSub one_sub = {};

  std::string _str;
  if (sub_str.back() == '\n')
    _str = sub_str;
  else
    _str = sub_str + '\n';
  auto vsub = split(_str, '\n');
  bool has_started_sub = false;
  std::string ssub;

  for (auto ssub : vsub)
  {
    if (ssub.empty())
    {
      if (has_started_sub)
        _vec.push_back(one_sub);
      one_sub = {};
      has_started_sub = false;
    }
    else
    {
      if (std::regex_match(ssub, sub_match, re_index))
      {
        if (sub_match.size() < 2)
          std::cerr << "Expected one sub index, ignoring error." << std::endl;
        else
        {
          one_sub.index = std::stoi(sub_match[1]);
          if (sub_match.size() > 2)
            std::cerr << "Expected only one sub index, got more, ignoring error." << std::endl;
        }
        has_started_sub = true;
      }
      else if (has_started_sub)
      {
        if (std::regex_match(ssub, sub_match, re_times))
        {
          if (sub_match.size() < 3)
            std::cerr << "Expected appearance and disappearance sub times, ignoring error." << std::endl;
          else
          {
            one_sub.appearance = str_to_ms(sub_match[1]);
            one_sub.disappearance = str_to_ms(sub_match[2]);
          }
          if (sub_match.size() > 3)
            std::cerr << "Expected appearance and disappearance sub times, got more, ignoring error." << std::endl;
        }
        else
        {
          one_sub.text += ssub + '\n';
        }
      }
    }
  }

  return _vec;
}

const std::regex cSub::re_index("(\\d+)");
const std::regex cSub::re_times("(\\d\\d:\\d\\d:\\d\\d.\\d\\d\\d)[^\\S\\n]+-->[^\\S\\n]+(\\d\\d:\\d\\d:\\d\\d.\\d\\d\\d)");

std::string cSub::to_str(bool dot)
{
  if (sub_vec.empty())
    return "";

  std::stringstream ss;
  for (size_t i = 0; i < sub_vec.size(); i++)
  {
    ss << i + 1 << std::endl;
    ss << ms_to_str(sub_vec[i].appearance, dot) << " --> " << ms_to_str(sub_vec[i].disappearance, dot) << std::endl;
    ss << sub_vec[i].text << std::endl;
  }

  return trim(ss.str());
}

int cSub::line_by_timestamp(int to_find, sSub &ssub)
{
  auto vsub = split(sub_str, '\n');
  std::string str_sub;
  std::smatch sub_match;
  int appearance, disappearance, last_disappearance = 0;
  int line_number = -1;

  int sub_count = 0, sub_index;

  for (int i = 0; i < (int)vsub.size(); i++)
  {
    str_sub = vsub[i];

    if (!str_sub.empty() && std::regex_match(str_sub, sub_match, re_times) && sub_match.size() >= 2)
    {
      appearance = str_to_ms(sub_match[1]);
      disappearance = str_to_ms(sub_match[2]);
      if (to_find >= last_disappearance && to_find <= appearance)
      {
        sub_index = sub_count - 1;
        if (sub_index < 0)
          sub_index = 0;

        if (i > 0)
        {
          line_number = i - (int)(2 + linecount(sub_vec[sub_index].text));
        }
        else
          line_number = 1;
      }
      if (to_find >= appearance && to_find <= disappearance)
      {
        line_number = i;
        sub_index = sub_count;
      }
      last_disappearance = disappearance;
      sub_count++;
    }
  }

  if (line_number == -1)
  {
    line_number = (int)vsub.size();
    sub_index = (int)sub_vec.size() - 1;
  }

  ssub = sub_vec[sub_index];
  ssub.index = sub_index;
  return (int)line_number;
}

size_t cSub::linecount(const std::string s)
{
  size_t nl = 1;
  const char *p = s.c_str();
  if (p)
    while (*p)
    {
      if (*p == '\n')
        nl++;
      p++;
    }
  return nl;
}

void cSub::parse(const std::string s)
{
  sub_str = s;
  sub_vec = to_vec();
  sub_str = to_str(dot);
  nlines = linecount(sub_str);
}

void cSub::parse(const char *s)
{
  if (s == nullptr)
    parse("");
  else
    parse(std::string(s));
}

bool cSub::diff(const std::string before_parse)
{
  parse(before_parse);
  return (before_parse != sub_str);
}

bool cSub::diff(const char *before_parse)
{
  std::string _before_parse = "";
  if (before_parse != nullptr)
    _before_parse = std::string(before_parse);

  return diff(_before_parse);
}

// For the current sub,
//  from a new begin and end stamps in milliseconds (provided as int),
//  compute new start offset, stop offset in milliseconds (returned as int) and coefficient number (returned as double)
void cSub::factors(const int begin_stamp, const int end_stamp, int &offset_start, int &offset_stop, double &coeff)
{
  int old_dur = sub_vec.back().appearance - sub_vec[0].appearance, new_dur = end_stamp - begin_stamp;
  offset_start = begin_stamp - sub_vec[0].appearance;
  offset_stop = end_stamp - sub_vec.back().appearance;
  if (old_dur == 0)
    coeff = 1;
  else
    coeff = (double)new_dur / (double)old_dur;
}

// begin_stamp and end_stamp in milliseconds. _offset_start and _offset_stop in floating point seconds. coeff being just a floating point number
bool cSub::adjust(const int time_start, const int time_end, const double _offset_start, const double _offset_stop, const double coeff)
{
  if (coeff <= 0)
  {
    err_msg = "Duration factor cannot be less or equal to zero";
    return false;
  }

  double offset_start = _offset_start * 1000;
  double offset_stop = _offset_stop * 1000;
  double new_time_start = offset_start + time_start;

  if (new_time_start > 356400000)
  {
    err_msg = "Begin limit would result in being greater than 99 hours";
    return false;
  }

  err_msg = "";

  // Cannot have a negative start time
  if (new_time_start < 0)
  {
    offset_start -= new_time_start;
    new_time_start = 0;
    err_msg = "Found a negative start time that has been reset to zero";
    logW(err_msg);
  }

  if (sub_vec.size() > 0)
  {
    double new_time_end = time_end;

    std::vector<sSub> new_sub_vec;
    // Approche itérative de la limite de fin désirée en fonction des offset et coeff, y-a peut-être une manière plus efficace (non itérative) de faire mais pour l'instant ça m'échappe ...
    do
    {
      double new_duration = coeff * ((offset_stop + new_time_end) - new_time_start);
      new_sub_vec.clear();

      double old_duration = sub_vec.back().appearance - sub_vec[0].appearance;
      double new_coeff = new_duration / old_duration;
      double new_disappearance = new_time_start + new_coeff * (sub_vec[0].disappearance - sub_vec[0].appearance);

      new_sub_vec.push_back({1, (int)std::trunc(new_time_start), (int)std::trunc(new_disappearance), sub_vec[0].text});

      if (sub_vec.size() > 1)
      {
        double new_appearance;
        int j = 1;

        for (size_t i = 1; i < sub_vec.size(); i++)
        {
          new_appearance = (double)new_sub_vec.back().appearance + new_coeff * (sub_vec[i].appearance - sub_vec[i - 1].appearance);
          new_disappearance = new_appearance + new_coeff * (sub_vec[i].disappearance - sub_vec[i].appearance);

          // Does not accept any timestamp greater than 99 hours
          if (new_appearance > 356400000 || new_disappearance > 356400000)
          {
            err_msg = "Cannot accept subtitle processing that would results having one of its timestamp greater than 99 hours";
            logE(err_msg);
            return false;
          }

          if (!trim(sub_vec[i].text).empty())
            new_sub_vec.push_back({j++, (int)std::trunc(new_appearance), (int)std::trunc(new_disappearance), sub_vec[i].text});
        }
      }

      sub_vec.clear();
      sub_vec = new_sub_vec;
      new_time_end++;
    } while (new_sub_vec.back().appearance < time_end);
    sub_str = to_str(dot);
    return true;
  }
  else
  {
    err_msg = "subtitle text is empty";
    logW(err_msg);
  }

  return false;
}

cSub::cSub(const std::string s, bool _dot)
{
  dot = _dot;
  err_msg = "";
  parse(s);
}
