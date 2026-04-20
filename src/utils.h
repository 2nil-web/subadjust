#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#define DIR_SEP "\\"
size_t my_wcstombs(char *, const wchar_t *, size_t);
#else
#define DIR_SEP "/"
#endif

#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/fl_types.h>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#define STRINGIFY(x) _STRINGIFY_HELPER(x)
#define _STRINGIFY_HELPER(x) #x

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define RANGE(x) std::begin(x), std::end(x)

#define HI_NYB(n) (uchar)(((n) & 0xF0) >> 4)
#define LO_NYB(n) (uchar)((n) & 0x0F)
#define LE16(n) (uchar)((n) & 0xFFUL), (uchar)(((n) & 0xFF00UL) >> 8)
#define LE32(n) (uchar)((n) & 0xFFUL), (uchar)(((n) & 0xFF00UL) >> 8), (uchar)(((n) & 0xFF0000UL) >> 16), (uchar)(((n) & 0xFF000000UL) >> 24)
#define BE16(n) (uchar)(((n) & 0xFF00UL) >> 8), (uchar)((n) & 0xFFUL)
#define BE32(n) (uchar)(((n) & 0xFF000000UL) >> 24), (uchar)(((n) & 0xFF0000UL) >> 16), (uchar)(((n) & 0xFF00UL) >> 8), (uchar)((n) & 0xFFUL)

typedef uint8_t size8_t;
typedef uint16_t size16_t;
typedef uint32_t size32_t;
typedef uint64_t size64_t;

std::string my_getenv(const std::string, bool msg_if_empty = false);
bool my_setenv(const std::string, const std::string);

bool starts_with_ignore_case(std::string_view s, std::string_view p);
bool ends_with_ignore_case(std::string_view s, std::string_view p);
void add_dot_ext(const char *f, const char *ext, char *s);
int text_width(const char *l, int pad = 0);
bool file_exists(const char *f);
size_t file_size(const char *f);
size_t file_size(FILE *f);
void open_ifstream(std::ifstream &ifs, const char *f);
bool check_read(FILE *file, uchar *expected, size_t n);
char *my_strerror(int errn);
std::string trim(const std::string &, std::string seps = "");
std::string trim(std::string &, std::string seps = "");
std::string rtrim(std::string &, std::string seps = "");
std::string ltrim(std::string &, std::string seps = "");
std::string to_upper(const std::string);
std::vector<std::string> split(const std::string &, const char);
void split_1st(std::string &r1, std::string &r2, std::string s, char c = ' ');
int str_to_ms(std::string str);
std::string pad(unsigned int d, size_t np = 2);
std::string ms_to_str(int millisec, bool dot = false);
std::string strict_replace(const std::string str, std::string subs, size_t pos = 0);
std::string replace_string_in_place(std::string &, const std::string &, const std::string &);
std::string replace_string(std::string, const std::string &, const std::string &);
std::string ISO8601(std::time_t);
std::string ISO8601_now();
std::string dup_anti_slash(const std::string);
const char *dup_anti_slash(const char *);
int process_id();
#endif
