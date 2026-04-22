#ifndef PLACE_H
#define PLACE_H
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// A class that create indexed files in a directory
class place
{
  int _number, max_index;
  std::filesystem::path directory;
  bool do_leave;

public:
  bool take_place();
  void reset();
  place(std::filesystem::path _directory = "", int _max_index = 100);
  int number();
  bool leave();
  void wont_leave();
  void will_leave();
  bool is_empty();
  ~place();
};

#endif /* PLACE_H */
