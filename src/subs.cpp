
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>

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

bool parse_1sub(std::string ssub, std::smatch sub_match, std::regex re_times, sSub &one_sub)
{
  if (std::regex_match(ssub, sub_match, re_times))
  {
    if (sub_match.size() < 3)
    {
      std::cerr << "Expected appearance and disappearance sub times, ignoring error." << std::endl;
      return false;
    }
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

  return true;
}

std::vector<sSub> cSub::to_vec(const std::string _sub_str)
{
  if (_sub_str.empty())
    return {};

  std::vector<sSub> _vec;
  std::smatch sub_match;
  sSub one_sub = {};

  std::string _str;
  if (_sub_str.back() == '\n')
    _str = _sub_str;
  else
    _str = _sub_str + '\n';

  auto vsub = split(_str, '\n');

  isvtt = false;

  for (auto ssub : vsub)
  {
    std::string s = ssub;
    trim(s);
    // logD("look vtt: ", s);
    if (!s.empty() && s != "WEBVTT")
      break;
    if (s == "WEBVTT")
      isvtt = true;
    vsub.erase(vsub.begin());
  }
  /*
    if (isvtt)
      logD("After look vtt: ISVTT");
    else
      logD("After look vtt: ISNOVTT");
    logD("After look vtt - vsub[0]: ", vsub[0]);
  */
  bool has_started_sub = false;
  std::string ssub;

  int ns = 1;
  for (auto _ssub : vsub)
  {
    auto ssub = _ssub;
    trim(ssub);

    if (ssub.empty())
    {
      if (has_started_sub)
      {
        // logD("ssub empty pushing back: ", one_sub.index, "\n", one_sub.appearance, ">>>", one_sub.disappearance, "\n", one_sub.text);
        _vec.push_back(one_sub);
      }
      one_sub = {};
      has_started_sub = false;
    }
    else
    {
      if (isvtt)
      {
        // logD("isvtt - ssub: ", ssub);

        if (parse_1sub(ssub, sub_match, re_times, one_sub))
        {
          // logD("isvtt - ssub: ", ssub);
          one_sub.index = ns++;
          has_started_sub = true;
        }
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
          parse_1sub(ssub, sub_match, re_times, one_sub);
        }
      }
    }
  }

  // logD("end to_vec: ");
  return _vec;
}

const std::regex cSub::re_index("(\\d+)");
// const std::regex cSub::re_times("(\\d\\d:\\d\\d:\\d\\d.\\d\\d\\d)[^\\S\\n]+-->[^\\S\\n]+(\\d\\d:\\d\\d:\\d\\d.\\d\\d\\d)");
const std::regex cSub::re_times("(.*:.*:.*..*)[^\\S\\n]+-->[^\\S\\n]+(.*:.*:.*..*)");

std::string cSub::to_str(const std::vector<sSub> vec, bool dot)
{
  if (vec.empty())
    return "";

  std::stringstream ss;
  for (size_t i = 0; i < vec.size(); i++)
  {
    ss << i + 1 << std::endl;
    ss << ms_to_str(vec[i].appearance, dot) << " --> " << ms_to_str(vec[i].disappearance, dot) << std::endl;
    ss << vec[i].text << std::endl;
  }

  return trim(ss.str());
}

std::string cSub::to_vtt_apart(const std::vector<sSub> vec)
{
  if (vec.empty())
    return "";

  std::stringstream ss;
  ss << "WEBVTT" << std::endl << std::endl;

  for (auto ev : vec)
  {
    ss << ms_to_str(ev.appearance, true) << " --> " << ms_to_str(ev.disappearance, true) << std::endl;
    std::string s = ev.text;
    trim(s);
    ss << s << std::endl;
  }

  return ss.str();
}

std::string cSub::to_vtt()
{
  return to_vtt_apart(sub_vec);
}

std::string cSub::to_sv_apart(const std::vector<sSub> vec, bool dot, const std::string sep, const std::string delim, bool with_bom)
{
  if (vec.empty())
    return "";

  const std::string sd = sep + delim, ds = delim + sep;

  std::stringstream ss;
  if (with_bom)
    ss << "\357\273\277";
  ss << "index" << sep << "start time" << sep << "end time" << sep << "text" << std::endl;

  for (size_t i = 0; i < vec.size(); i++)
  {
    ss << i + 1 << sd;
    if (sep == ";")
    {
      ss << vec[i].appearance << ds << delim << vec[i].disappearance << ds;
    }
    else
    {
      ss << ms_to_str(vec[i].appearance, dot) << ds << delim << ms_to_str(vec[i].disappearance, dot) << ds;
    }
    std::string s = vec[i].text;
    s = trim(s);
    s = replace_string(s, delim, delim + delim);
    //    s=replace_string(s, "\'", "\\\'");
    ss << delim << s << delim << std::endl;
  }

  return ss.str();
}

std::string cSub::to_sv(std::string sep, std::string delim)
{
  return to_sv_apart(sub_vec, dot, sep, delim);
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

void cSub::parse_apart(std::string &s, std::vector<sSub> &v, size_t &nl)
{
  v = to_vec(s);
  s = to_str(v, dot);
  nl = linecount(s);
}

void cSub::parse(const std::string s)
{
  sub_str = s;
  parse_apart(sub_str, sub_vec, nlines);
  // logD("aft call parse_apart");
}

void cSub::parse(const char *s)
{
  if (s == nullptr)
    parse("");
  else
    parse(std::string(s));
}

// Find and return the closest appearance and disappearance to app and dis in v
void cSub::find_closest_times(int &app, int &dis, const std::vector<sSub> v)
{
  constexpr unsigned int max_uint{
#ifdef _MSVC_LANG
      UINT_MAX
#else
      std::numeric_limits<unsigned int>::max()
#endif
  };

  int _app = app, _dis = dis;
  unsigned int diff_app = max_uint, diff_dis = max_uint;

  for (auto s : v)
  {
    unsigned int curr_diff_app = std::abs(app - s.appearance);

    if (diff_app > curr_diff_app)
    {
      diff_app = curr_diff_app;
      _app = s.appearance;
    }

    unsigned int curr_diff_dis = std::abs(dis - s.disappearance);

    if (diff_dis > curr_diff_dis)
    {
      diff_dis = curr_diff_dis;
      _dis = s.disappearance;
    }
  }

  app = _app;
  dis = _dis;
}

// Return true if sync was able to aligns the subtitles contained within s1 with those contained within s2, else false
bool cSub::sync(const std::string _s1, const std::string _s2)
{
  // They are strictly equals, so nothing changes
  if (_s1 == _s2)
    return false;

  std::vector<sSub> v1;
  std::string s1 = _s1;
  size_t nl1;

  parse_apart(s1, v1, nl1);

  std::vector<sSub> v2;
  std::string s2 = _s2;
  size_t nl2;
  parse_apart(s2, v2, nl2);

  // Firstly, linearly adjust v1 to v2
  adjust_apart(s1, v1, v2[0].appearance, v2.back().appearance);

  // Then align each of the timestamp of v1 to their closest inferior and closest superior or equal counterpart in v2
  // Pour chaque timestamp appearance et disappearance (v1[i1].app et v1[i1].dis, sauf le 1er et le dernier) du fichier à synchroniser, dans le fichier de référence prend :
  //  Le timestamp immédiatement inférieur et celui immédiatement supérieur ou égal
  //  Et calcul leur moyenne (inf+v1[i1]+sup)/3 qu'elle applique à v1[i1]
  for (size_t i1 = 1; i1 < v1.size() - 1; i1++)
  {
    int new_app = -1, new_dis = -1;

    for (size_t i2 = 1; i2 < v2.size(); i2++)
    {
      if (v1[i1].appearance > v2[i2 - 1].appearance && v1[i1].appearance <= v2[i2].appearance)
      {
        new_app = (v2[i2 - 1].appearance + v1[i1].appearance + v2[i2].appearance) / 3;
        // Avoid to have various subtitles with interweave appearance timestamp
        if (new_app <= v1[i1 - 1].appearance)
          new_app = (v2[i2 - 1].appearance + v1[i1 - 1].appearance + v1[i1].appearance + v2[i2].appearance) / 4;
      }

      if (v1[i1].disappearance > v2[i2 - 1].disappearance && v1[i1].disappearance <= v2[i2].disappearance)
      {
        new_dis = (v2[i2 - 1].disappearance + v1[i1].disappearance + v2[i2].disappearance) / 3;
        // Avoid to have various subtitles with interweave DISappearance timestamp
        if (new_dis <= v1[i1 - 1].disappearance)
          new_dis = (v2[i2 - 1].disappearance + v1[i1 - 1].disappearance + v1[i1].disappearance + v2[i2].disappearance) / 4;
      }

      if (new_app != -1 && new_dis != -1)
        break;
    }

    // Avoid to have various subtitles with interweave timestamp
    if (new_dis < v1[i1 - 1].disappearance)
      new_dis = v1[i1 - 1].disappearance;
    if (new_app > new_dis)
      new_app = (v1[i1 - 1].disappearance + new_dis) / 2;

    v1[i1].appearance = new_app;
    v1[i1].disappearance = new_dis;
  }

  s1 = to_str(v1, dot);
  // Nothing has changed with the syncing
  if (s1 == _s1)
    return false;

  // logD("CSUB SYNC");
  //  Something has changed with the syncing
  sub_vec = v1;
  sub_str = to_str(sub_vec, dot);
  nlines = linecount(sub_str);

  return true;
}

bool cSub::sync(const char *s1, const char *s2)
{
  if (s1 && s2)
    return sync(std::string(s1), std::string(s2));

  return false;
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
bool cSub::adjust_apart(std::string &_str, std::vector<sSub> &_vec, const int time_start, const int time_end, const double _offset_start, const double _offset_stop, const double coeff)
{
  // logD("adjust_apart: start");

  if (coeff <= 0)
  {
    err_msg = "Duration factor cannot be less or equal to zero";
    logE("adjust_apart1: false", err_msg);
    return false;
  }

  double offset_start = _offset_start * 1000;
  double offset_stop = _offset_stop * 1000;
  double new_time_start = offset_start + time_start;

  if (new_time_start > 356400000)
  {
    err_msg = "Begin limit would result in being greater than 99 hours";
    logE("adjust_apart2: false", err_msg);
    return false;
  }

  err_msg = "";

  // Cannot have a negative start time
  if (new_time_start < 0)
  {
    offset_start -= new_time_start;
    new_time_start = 0;
    err_msg = "Found a negative start time that has been reset to zero";
    logW("adjust_apart1: false", err_msg);
  }

  if (_vec.size() > 0)
  {
    double new_time_end = time_end;

    std::vector<sSub> new_sub_vec;
    // Approche itérative de la limite de fin désirée en fonction des offset et coeff, y-a peut-être une manière plus efficace (non itérative) de faire mais pour l'instant ça m'échappe ...
    do
    {
      double new_duration = coeff * ((offset_stop + new_time_end) - new_time_start);
      new_sub_vec.clear();

      double old_duration = _vec.back().appearance - _vec[0].appearance;
      double new_coeff = new_duration / old_duration;
      double new_disappearance = new_time_start + new_coeff * (_vec[0].disappearance - _vec[0].appearance);

      new_sub_vec.push_back({1, (int)std::trunc(new_time_start), (int)std::trunc(new_disappearance), _vec[0].text});

      if (_vec.size() > 1)
      {
        double new_appearance;
        int j = 1;

        for (size_t i = 1; i < _vec.size(); i++)
        {
          new_appearance = (double)new_sub_vec.back().appearance + new_coeff * (_vec[i].appearance - _vec[i - 1].appearance);
          new_disappearance = new_appearance + new_coeff * (_vec[i].disappearance - _vec[i].appearance);

          // Does not accept any timestamp greater than 99 hours
          if (new_appearance > 356400000 || new_disappearance > 356400000)
          {
            err_msg = "Cannot accept subtitle processing that would results having one of its timestamp greater than 99 hours";
            logE("adjust_apart3: false", err_msg);
            return false;
          }

          if (!trim(_vec[i].text).empty())
            new_sub_vec.push_back({j++, (int)std::trunc(new_appearance), (int)std::trunc(new_disappearance), _vec[i].text});
        }
      }

      _vec.clear();
      _vec = new_sub_vec;
      new_time_end++;
    } while (new_sub_vec.back().appearance < time_end);
    _str = to_str(_vec, dot);

    // logD("adjust_apart5: true");
    return true;
  }
  else
  {
    err_msg = "subtitle text is empty";
    logW("adjust_apart2: ", err_msg);
  }

  logW("adjust_apart3: false", err_msg);
  return false;
}

bool cSub::adjust(const int time_start, const int time_end, const double offset_start, const double offset_stop, const double coeff)
{
  // logD("adjust:");
  return adjust_apart(sub_str, sub_vec, time_start, time_end, offset_start, offset_stop, coeff);
}

cSub::cSub(const std::string s, bool _dot)
{
  dot = _dot;
  err_msg = "";
  parse(s);
}
