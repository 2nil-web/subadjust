
#include <iostream>
#include <limits>
#include <random>
#include <string>

// Convert an int to a string padding on the left with np zeros
std::string pad(unsigned int d, size_t np = 2)
{
  std::string s = std::to_string(d);
  for (size_t i = s.size(); i < np; i++)
    s = '0' + s;

  return s;
}

// Convert a int in millisecond into a string of the format HH:MM:SS[.|,]SSS to
std::string ms_to_str(int millisec, bool dot = true)
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

int current_start = 0, n_subs = 1500, interval_average = 9400, interval_gap = 60000, length_average = 3000, length_gap = 10000;

int rnd_max(int max)
{
  std::random_device r;
  std::default_random_engine e(r());
  std::uniform_int_distribution<int> uniform_dist(0, max);
  return uniform_dist(e);
}

std::string a_subtitle(int n)
{
  int length = 300 + rnd_max(length_average) + rnd_max(length_gap);
  int current_end = current_start + length;
  std::string res = std::to_string(n + 1) + '\n' + ms_to_str(current_start) + " --> " + ms_to_str(current_end) + "\nToujours les même mots, on s\'en fout ...\n";
  current_start += 300 + length + rnd_max(interval_average) + rnd_max(interval_gap);
  return res;
}

int main(int argc, char **argv)
{
  std::vector<std::string> args(argv, argv + argc);

  if (args.size() > 1)
    current_start = std::stoi(args[1]);
  if (args.size() > 2)
    current_start = std::stoi(args[2]);
  if (args.size() > 3)
    interval_average = std::stoi(args[3]);
  if (args.size() > 4)
    interval_gap = std::stoi(args[4]);
  if (args.size() > 5)
    length_average = std::stoi(args[5]);
  if (args.size() > 6)
    length_gap = std::stoi(args[6]);

  for (int i = 0; i < n_subs; i++)
  {
    std::cout << a_subtitle(i) << std::endl;
  }
}
