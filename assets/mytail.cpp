
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifdef __OSX__
const char delimiter = '\r';
#else
const char delimiter = '\n';
#endif

// Return nullptr if unable to read the whole stream in mem
// Else buffer must be deleted, thereafter
char *stream_in_mem(std::istream &is, std::intmax_t &size)
{
  size = is.tellg();
  is.seekg(0, std::ios::beg);
  char *buf = new char[size + 1];
  // If not enough memory, then unuseful to read the file
  if (buf != nullptr)
    is.read(buf, size);
  // If ok buf will not be nullptr
  return buf;
}

// Return nullptr if unable to read the whole file in mem
// Else buffer must be deleted, thereafter
char *file_in_mem(std::filesystem::path filepath, std::intmax_t &size)
{
  if (filepath.string() == "-")
    return stream_in_mem(std::cin, size);

  char *s = nullptr;
  std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);

  if (ifs)
  {
    s = stream_in_mem(ifs, size);
    ifs.close();
  }

  return s;
}

// Get file size and line count
char *count_line(std::filesystem::path filepath, std::intmax_t &fsize, std::intmax_t &nw, std::intmax_t &nl)
{
  char *s = file_in_mem(filepath, fsize);

  if (s == nullptr)
    return nullptr;

  nl = nw = 0;

  for (uintmax_t i = 0; i < fsize; i++)
  {
    if (s[i] == delimiter)
      nl++;
    if (isspace(s[i]))
      nw++;
  }

  return s;
}

void tail_stream(std::istream &is, const std::intmax_t nl2read, std::intmax_t nlread, std::string &buf)
{
  is.seekg(-1, std::ios::end);
  char ch;
  nlread = 0;
  buf = "";

  for (std::intmax_t i = 0; i < is.tellg(); i++)
  {
    ch = is.get();

    if (ch == delimiter)
    {
      if (nlread < nl2read)
        nlread++;
      else
        break;
    }

    buf.insert(0, 1, ch);
    is.seekg(-2, std::ios::cur);
  }
}

bool tail(std::filesystem::path filepath, const std::intmax_t nl2read, std::intmax_t nlread, std::string &buf)
{
  std::uintmax_t fsize;

  if (filepath.string() == "-")
  {
    tail_stream(std::cin, nl2read, nlread, buf);
  }
  else
  {
    std::ifstream ifs(filepath, std::ifstream::ate | std::ifstream::binary);

    if (ifs)
    {
      tail_stream(ifs, nl2read, nlread, buf);
      ifs.close();
    }
    else
      return false;
  }

  return true;
}

int main(int argc, char **argv)
{
  char run_func = 't';
  std::vector<std::string> args(argv + 1, argv + argc);

  std::filesystem::path path = "-";
  if (args.size() > 0)
  {
    path = args[0];
    args.erase(args.begin());
  }

  if (args.size() > 0 && args[0] == "-c")
  {
    args.erase(args.begin());
    std::intmax_t fsize, nl;
    count_line(path, fsize, nl);
    std::cout << path << " - file size: " << fsize << ", line count: " << nl << std::endl;
    return 0;
  }

  std::uintmax_t nl2read = 10, nlread;
  std::string buf;
  if (args.size() > 1)
    nl2read = std::stoll(args[1]);

  tail(path, nl2read, nlread, buf);
  std::cout << buf << std::flush;

  return 0;
}
