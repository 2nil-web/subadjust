#ifdef _WIN32
// clang-format off
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#ifndef __unix__
#include <versionhelpers.h>
#endif
// clang-format on
#else
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cctype>
#include <chrono>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>

#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/filename.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#include "log.h"
#include "utils.h"

#ifdef _WIN32
std::tm *localtime_r(const time_t *tt, std::tm *tms)
{
  localtime_s(tms, tt);
  return tms;
}
#endif

// Get env var
// Return true if exists else empty string
std::string my_getenv(const std::string var, bool msg_if_empty)
{
  char *pVal = nullptr;
  std::string sVal = "";
  if (msg_if_empty)
    sVal = "No value found for " + var;

#ifdef _WIN32
  DWORD vl = GetEnvironmentVariableA(var.c_str(), NULL, 0);
  if (vl > 0)
  {
    pVal = new char[vl + 1];
    GetEnvironmentVariableA(var.c_str(), pVal, vl);
    if (pVal != nullptr)
    {
      sVal = pVal;
      delete[] pVal;
    }
  }
#else
  pVal = getenv(var.c_str());
  if (pVal != nullptr)
    sVal = pVal;
#endif

  return sVal;
}

std::filesystem::path personal_dir()
{
  std::filesystem::path var;
#ifdef _WIN32
  var = my_getenv("USERPROFILE");
#else
  var = my_getenv("HOME");
#endif

  return var;
}

// Set env var
// Return true if OK else false
bool my_setenv(const std::string var, const std::string val)
{
#ifdef _WIN32
  return SetEnvironmentVariableA(var.c_str(), val.c_str()) != 0;
#else
  return setenv((char *)var.c_str(), (char *)val.c_str(), 1) == 0;
#endif
}

bool create_directory_if_possible(std::filesystem::path p)
{
  if (std::filesystem::is_directory(p))
    return true;
  else
    return std::filesystem::create_directory(p);
}

bool create_file(std::filesystem::path p)
{
  std::ofstream f(p);
  if (f)
  {
    f.close();
    return true;
  }
  else
    return false;
}

bool create_file_if_possible(std::filesystem::path p)
{
  if (std::filesystem::exists(p))
    return false;
  else
    return create_file(p);
}

std::filesystem::path admin_file(std::filesystem::path filename, std::filesystem::path parent_dir)
{
  std::filesystem::path p;
  create_directory_if_possible(parent_dir);

#ifdef _WIN32
  int attr = GetFileAttributes(p.wstring().c_str());
  if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
  {
    SetFileAttributes(p.wstring().c_str(), attr | FILE_ATTRIBUTE_HIDDEN);
  }
#endif
  p = parent_dir / filename;
  logD("admin_file: ", p);
  return p;
}

#ifdef _WIN32
char *my_strcpy(char *dst, const char *src)
{
  size_t l = strlen(src);
  strcpy_s(dst, l, src);
  return dst;
}
char *my_strcat(char *dst, const char *src)
{
  size_t l = strlen(src);
  strcat_s(dst, l, src);
  return dst;
}

size_t my_wcstombs(char *mbstr, const wchar_t *wcstr, size_t count)
{
  size_t sz = count, pret;

  return wcstombs_s(&pret, mbstr, sz, wcstr, count);
}

char *my_strerror(int errn)
{
  static char buf[1024];
  strerror_s(buf, 1024, errn);
  return buf;
}
#else
char *my_strerror(int errn)
{
  return strerror(errn);
}

char *my_strcpy(char *dst, const char *src)
{
  return strcpy(dst, src);
}
char *my_strcat(char *dst, const char *src)
{
  return strcat(dst, src);
}
#endif

static bool cmp_ignore_case(const char &a, const char &b)
{
  return tolower(a) == tolower(b);
}

bool starts_with_ignore_case(std::string_view s, std::string_view p)
{
  if (s.size() < p.size())
  {
    return false;
  }
  std::string_view ss = s.substr(0, p.size());
  return std::equal(RANGE(ss), RANGE(p), cmp_ignore_case);
}

bool ends_with_ignore_case(std::string_view s, std::string_view p)
{
  if (s.size() < p.size())
  {
    return false;
  }
  std::string_view ss = s.substr(s.size() - p.size());
  return std::equal(RANGE(ss), RANGE(p), cmp_ignore_case);
}

void add_dot_ext(const char *f, const char *ext, char *s)
{
  my_strcpy(s, f);
  const char *e = fl_filename_ext(s);
  if (!e || !strlen(e))
  {
    my_strcat(s, ext);
  }
}

int text_width(const char *l, int pad)
{
  int lw = 0, lh = 0;
  fl_measure(l, lw, lh, 0);
  return lw + 2 * pad;
}

bool file_exists(const char *f)
{
  return !fl_access(f, 4); // R_OK
}

size_t file_size(const char *f)
{
  struct stat s;
  int r = fl_stat(f, &s);
  return r ? 0 : (size_t)s.st_size;
}

size_t file_size(FILE *f)
{
#ifdef __CYGWIN__
#define stat64 stat
#define fstat64 fstat
#elif defined(_WIN32)
#define fileno _fileno
#define stat64 _stat32i64
#define fstat64 _fstat32i64
#endif
  struct stat64 s;
  int r = fstat64(fileno(f), &s);
  return r ? 0 : (size_t)s.st_size;
}

void open_ifstream(std::ifstream &ifs, const char *f)
{
#ifdef _WIN32
  wchar_t wf[FL_PATH_MAX] = {};
  fl_utf8towc(f, (unsigned int)strlen(f), wf, sizeof(wf));
  ifs.open(wf);
#else
  ifs.open(f);
#endif
}

bool check_read(FILE *file, uchar *expected, size_t n)
{
  std::vector<uchar> buffer(n);
  size_t r = fread(buffer.data(), 1, n, file);
  return r == n && (!expected || !memcmp(buffer.data(), expected, n));
}

std::string ws_str()
{
  static std::string ws;

  if (ws.empty())
  {
    std::locale loc;
    for (char c = (std::numeric_limits<char>::min)(); c < (std::numeric_limits<char>::max)(); c++)
      if (std::isspace(c, loc))
        ws += c;
    if (std::isspace((std::numeric_limits<char>::max)(), loc))
      ws += (std::numeric_limits<char>::max)();
  }

  return ws;
}

std::string trim(std::string &s, std::string _seps)
{
  std::string seps;
  if (_seps == "")
    seps = ws_str();
  else
    seps = _seps;

  size_t start = s.find_first_not_of(seps);
  if (start == std::string::npos)
    return "";
  return s.substr(start, s.find_last_not_of(seps) - start + 1);
}

std::string trim(const std::string &_s, std::string _seps)
{
  std::string s = _s;
  return trim(s, _seps);
}

std::string ltrim(std::string &s, std::string seps)
{
  //  std::string seps;
  if (seps == "")
    seps = ws_str();
  //  else seps=_seps;

  s.erase(0, s.find_first_not_of(seps));
  return s;
}

std::string rtrim(std::string &s, std::string ws)
{
  s.erase(s.find_last_not_of(ws) + 1);
  return s;
}

std::string to_upper(const std::string cs)
{
  std::string s = cs;
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

std::vector<std::string> split(const std::string &s, const char c)
{
  std::string::size_type i = 0, j = s.find(c);
  std::vector<std::string> v;

  while (j != std::string::npos)
  {
    v.push_back(s.substr(i, j - i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }

  return v;
}

void split_1st(std::string &r1, std::string &r2, std::string s, char c)
{
  size_t pos = s.find_first_of(c);
  if (pos == std::string::npos)
  {
    r1 = s;
    r2 = {};
  }
  else
  {
    r1 = s.substr(0, pos);
    trim(r1);
    r2 = s.substr(pos + 1);
    trim(r2);
  }
}

// Convert a string of the format HH:MM:SS[.|,]SSS to a int in millisecond
int str_to_ms(std::string str)
{
  int h = 0, m = 0, s = 0, ms = 0;
  if (str.size() >= 2)
    h = std::stoi(str.substr(0, 2));
  if (str.size() >= 5)
    m = std::stoi(str.substr(3, 2));
  if (str.size() >= 8)
    s = std::stoi(str.substr(6, 2));
  if (str.size() >= 12)
    ms = std::stoi(str.substr(9, 3));
  int millisec = (h * 3600 + m * 60 + s) * 1000 + ms;
  return millisec;
}

// Convert an int to a string padding on the left with np zeros
std::string pad(unsigned int d, size_t np)
{
  std::string s = std::to_string(d);
  for (size_t i = s.size(); i < np; i++)
    s = '0' + s;

  return s;
}

// Convert a int in millisecond into a string of the format HH:MM:SS[.|,]SSS to
std::string ms_to_str(int millisec, bool dot)
{
  bool neg = false;
  if (millisec < 0)
  {
    millisec = -millisec;
    neg = true;
  }

  int keep_ms, keep_sec, keep_mn;

  int sec = millisec / 1000;
  keep_ms = millisec % 1000; // millisec à garder

  int mn = sec / 60;
  keep_sec = sec % 60; // sec à garder

  int hr = mn / 60;  // hr à garder
  keep_mn = mn % 60; // mn à garder

  std::string str = pad(hr) + ':' + pad(keep_mn) + ':' + pad(keep_sec);
  if (dot)
    str += '.';
  else
    str += ',';
  str += pad(keep_ms, 3);

  if (neg)
    str = "-" + str;
  return str;
}

// Replace a portion of the string str, starting at pos, by substring subs not growing str
std::string strict_replace(const std::string str, std::string subs, size_t pos)
{
  std::string s = str;
  size_t i, i2;

  if (pos < s.size())
  {
    for (i = 0; i < subs.size(); i++)
    {
      i2 = pos + i;
      if (i2 < s.size())
        s[i2] = subs[i];
    }
  }

  return s;
}

std::string replace_string_in_place(std::string &s, const std::string &srch, const std::string &rep)
{
  size_t pos = 0;
  while ((pos = s.find(srch, pos)) != std::string::npos)
  {
    s.replace(pos, srch.length(), rep);
    pos += rep.length();
  }

  return s;
}

std::string replace_string(std::string s, const std::string &srch, const std::string &rep)
{
  size_t pos = 0;
  while ((pos = s.find(srch, pos)) != std::string::npos)
  {
    s.replace(pos, srch.length(), rep);
    pos += rep.length();
  }
  return s;
}

std::string ISO8601(const std::time_t tt)
{
  std::tm tms;
  localtime_r(&tt, &tms);
  // Avec put_time %z donne des resultats aleatoire sous windows/msys2
#ifdef _WIN32
#define TIME_FORMAT "%Y-%m-%dT%H:%M:%S"
#else
#define TIME_FORMAT "%Y-%m-%dT%H:%M:%S%z"
#endif
  std::stringstream buffer;
  buffer << std::put_time(&tms, TIME_FORMAT);
  return buffer.str();
}

std::string ISO8601_now()
{
  return ISO8601(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

std::string dup_anti_slash(const std::string s)
{
  std::string s2 = "";

  for (size_t i = 0; i < s.size(); i++)
  {
    if (s[i] == '\\')
      s2 += '\\';
    s2 += s[i];
  }

  return s2;
}

const char *dup_anti_slash(const char *s)
{
  return dup_anti_slash(std::string(s)).c_str();
}

int process_id()
{
#ifdef _WIN32
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif
}
