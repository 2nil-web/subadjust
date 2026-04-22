
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "place.h"
#include "utils.h"

// A class that create indexed files in a directory
bool place::take_place()
{
  std::filesystem::path file = directory / "0";
  _number = 0;

  if (std::filesystem::is_directory(directory))
  {
    int i = 0;
    while (i < max_index)
    {
      file = directory / std::to_string(i);
      if (!std::filesystem::exists(file))
        break;
      i++;
    }

    if (i >= max_index)
    {
      _number = -1;
      return false;
    }

    _number = i;
  }
  else
    std::filesystem::create_directory(directory);

  return create_file(file);
}

void place::reset()
{
  if (std::filesystem::is_directory(directory))
    std::filesystem::remove_all(directory);
}

place::place(std::filesystem::path _directory, int _max_index)
{
  if (_directory.empty())
    directory = personal_dir() / ".places";
  else
    directory = _directory;

  max_index = _max_index;
  take_place();
  do_leave = true;
}

bool place::leave()
{
  if (do_leave)
  {
    std::filesystem::path file(directory / std::to_string(_number));
    if (std::filesystem::exists(file) && std::filesystem::is_regular_file(file))
    {
      std::filesystem::remove(file);
    }

    if (std::filesystem::is_directory(directory) && std::filesystem::is_empty(directory))
      std::filesystem::remove_all(directory);
    return true;
  }

  return false;
}

int place::number()
{
  return _number;
}

void place::wont_leave()
{
  do_leave = false;
}

void place::will_leave()
{
  do_leave = true;
}

place::~place()
{
  leave();
}

bool place::is_empty()
{
  if (!std::filesystem::exists(directory) || (std::filesystem::is_directory(directory) && std::filesystem::is_empty(directory)))
    return true;
  return false;
}

#ifdef USE_EXAMPLE
int main(int argc, char **argv)
{
  place mr;

  std::cout << "Enter \"no\" to quit without leaving place, \"reset\" to clean all the places else simply <ENTER>: " << std::flush;
  std::string rep;
  getline(std::cin, rep);
  if (rep == "no")
    mr.wont_leave();
  else if (rep == "reset")
    mr.reset();
  return 0;
}
#endif
