
#include <filesystem>
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
std::string ms_to_str(int millisec, bool dot = false)
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

int rnd(int min, int max)
{
  std::random_device r;
  std::default_random_engine e(r());
  std::uniform_int_distribution<int> uniform_dist(min, max);
  return uniform_dist(e);
}

int start, length, nsubs, ave_dur, curr_start, curr_end;
std::string a_subtitle(int n, std::string s = "Unimportant blah blah...")
{
  curr_end = curr_start + rnd(ave_dur - rnd(ave_dur / 10, 5 * ave_dur / 10), ave_dur + rnd(ave_dur / 10, 5 * ave_dur / 10));
  std::string ret = std::to_string(n + 1) + '\n' + ms_to_str(curr_start) + " --> " + ms_to_str(curr_end) + "\n" + s + "\n";
  curr_start = curr_end + rnd(ave_dur - rnd(ave_dur / 10, 5 * ave_dur / 10), ave_dur + rnd(ave_dur / 10, 5 * ave_dur / 10));

  return ret;
}

/*
./gen_subs.exe >sub.srt
./gen_subs.exe 94500 5243000 1678 >sub.srt
*/
int main(int argc, char **argv)
{
  std::vector<std::string> args(argv, argv + argc);

  if (args[1] == "-h" || args[1] == "-help" || args[1] == "--help")
  {
    std::cout << "Usage: " << std::filesystem::path(args[0]).stem().string() << " [ARGUMENTS]";
    std::cout << R"EOF(
Generate a subtitle file in srt format.
Arguments must respect the following order and are in millisecond when about time:
Start time (default from 0 to 2 minutes).
Duration of subtitles (default from 30 minutes to 3 hours).
Count of subtitles in the file (default from 400 to 2000).
)EOF";
  }
  else
  {
    if (args.size() > 1)
      start = std::stoi(args[1]);
    // Sinon entre 0 et 2 minutes (en ms)
    else
      start = rnd(0, 120000);

    curr_start = start;

    if (args.size() > 2)
      length = std::stoi(args[2]);
    // Sinon entre 30 minutes et 3 heures (en ms)
    else
      length = rnd(1800000, 10800000);

    if (args.size() > 3)
      nsubs = std::stoi(args[3]);
    // Sinon entre 400 et 2000 sous-titres
    else
      nsubs = rnd(400, 2000);

    ave_dur = length / (2 * nsubs);

    int i;
    for (i = 0; i < nsubs - 1; i++)
    {
      std::cout << a_subtitle(i) << std::endl;
    }

    std::cout << a_subtitle(i, "Stats:") << "  Start = " << ms_to_str(start) << "\n  Duration = " << ms_to_str(curr_end - start) << "\n  Subtitle count = " << nsubs << "\n  Average duration of a subtitle = " << ms_to_str(ave_dur) << std::endl;
  }
}
