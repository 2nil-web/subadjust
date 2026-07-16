#ifndef MY_WHICH_H
#define MY_WHICH_H
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef MAIN_RUN
std::string my_getenv(const std::string var, bool msg_if_empty = false);
std::vector<std::string> split(const std::string &str, const char delim);
#else
#include "utils.h"
#endif

bool which(std::string cmd, std::filesystem::path &res);

#endif /* MY_WHICH_H */
