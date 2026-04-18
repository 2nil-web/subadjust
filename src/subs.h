
#ifndef SUBS_H
#define SUBS_H

#include <regex>
#include <string>
#include <vector>

#include "utils.h"

struct sSub
{
  int index;
  // In milliseconds
  int appearance, disappearance;
  std::string text;
};

class cSub
{
private:
  static const std::regex re_index;
  static const std::regex re_times;
  std::vector<sSub> to_vec();
  std::string to_str(bool dot);
  size_t linecount(const std::string);
  std::string sub_str;
  size_t nlines;
  std::vector<sSub> sub_vec;

public:
  bool dot;
  size_t line_number();
  std::string str();
  const char *c_str();
  std::vector<sSub> vec();
  std::string err_msg;

  // Parse parameter and store resulting vector and string
  void parse(const std::string);
  void parse(const char *);
  // Compute the sub parameters
  void factors(const int begin_stamp, const int end_stamp, int &offset_start, int &offset_stop, double &coeff);
  // Adjust the sub according to the passed parameters
  bool adjust(const int, const int, const double, const double, const double);

  // Return true if parse result string is different from parameter
  bool diff(const std::string);
  bool diff(const char *);

  // Return the line number containing the time closest to the parameter
  int line_by_timestamp(int, sSub &);

  cSub(const std::string s = "", bool dot = false);
};

#endif /* SUBS_H */
