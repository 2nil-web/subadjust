
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef MAIN_RUN
std::string my_getenv(const std::string var, bool msg_if_empty = false)
{
  char *pVal = nullptr;
  std::string sVal = "";
  if (msg_if_empty)
    sVal = "No value found for " + var;

#ifdef _WIN32
  DWORD vl = GetEnvironmentVariable(var.c_str(), NULL, 0);
  if (vl > 0)
  {
    pVal = new char[vl + 1];
    GetEnvironmentVariable(var.c_str(), pVal, vl);
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

std::vector<std::string> split(const std::string &str, const char delim)
{
  std::vector<std::string> strings;
  size_t start;
  size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
  {
    end = str.find(delim, start);
    strings.push_back(str.substr(start, end - start));
  }
  return strings;
}
#else
#include "utils.h"
#endif

const char PATHSEP =
#ifdef _WIN32
    ';'
#else
    ':'
#endif
    ;

bool which(std::string cmd, std::filesystem::path &res)
{
  static std::vector<std::string> path_val_vec = {};
  if (path_val_vec.empty())
  {
    std::string path_val = my_getenv("PATH", false);
    // std::cout << "PATH: " << path_val << std::endl;
    path_val_vec = split(path_val, PATHSEP);
  }

  bool ret = false;
  std::filesystem::path cpth = {};

  for (auto p : path_val_vec)
  {
    std::filesystem::path cpth = std::filesystem::path(p) / std::filesystem::path(cmd);
    if (std::filesystem::exists(cpth)
#ifdef _WIN32
        || std::filesystem::exists(std::filesystem::path(p) / std::filesystem::path(cmd + ".exe"))
#endif
    )
    {
      if (res.empty())
        res = cpth;
      ret = true;
    }
  }

  return ret;
}

#ifdef MAIN_RUN
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR lpCmdLine, int /*nCmdShow*/)
#else
int main(int argc, char **argv)
#endif
{
  std::filesystem::path pth;

#ifdef _WIN32
  if (which(lpCmdLine, pth))
#else
  if (argc >= 1 && which(argv[1], pth))
#endif
  {
    std::cout << pth << std::endl;
    return 0;
  }
  else
    return EINVAL;
}
#endif
