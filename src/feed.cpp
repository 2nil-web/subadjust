
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

#include "feed.h"
#include "utils.h" // For my_getenv, personal_dir, create_file_if_possible, create_directory_if_possible

feed::feed(std::string feed_name, std::string feed_stop, int _delay_between_feed)
{
  feed_dir = personal_dir() / feed_name;
  file_to_stop_feed = feed_dir / feed_stop;
  delay_between_feed = _delay_between_feed;
}

bool feed::start()
{
  create_directory_if_possible(feed_dir);
  file_to_feed = feed_dir / "0";
  if (std::filesystem::is_empty(feed_dir) && create_file_if_possible(file_to_feed))
  {

    std::thread([&] {
      std::filesystem::path previous_file_to_feed = file_to_feed;
      int feed_index = 1;

      // While not asking to stop feed
      while (!std::filesystem::exists(file_to_stop_feed))
      {
        file_to_feed = feed_dir / std::to_string(feed_index);
        // Create file_to_feed
        create_file_if_possible(file_to_feed);
        // Remove previous_file_to_feed
        std::filesystem::remove(previous_file_to_feed);
        // previous_file_to_feed becomes file_to_feed
        previous_file_to_feed = file_to_feed;
        // Increment feed_index
        feed_index++;
        // And sleep delay_between_feed
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
      }
    }).detach();

    return true;
  }

  return false;
}

bool feed::stop()
{
  // Notice thread to stop
  bool ret = create_file_if_possible(file_to_stop_feed);
  // Delay to insure thread stopped
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
  // Then recursiveley remove feed_dir
  std::filesystem::remove_all(feed_dir);
  return ret;
}

// Return true if no pending feed
bool feed::is_being_fed()
{
  if (!std::filesystem::is_directory(feed_dir))
    return false;
  return !std::filesystem::is_empty(feed_dir);
}

#ifdef USE_EXAMPLES
int main(int argc, char **argv)
{
  feed mf;

  int loop_max = 10;
  if (argc > 1)
    loop_max = std::stoi(argv[1]);
  if (mf.start())
  {
    for (int i = 0; i < loop_max; i++)
    {
      std::cout << "main thread waiting: " << i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    mf.stop();
  }
  else
  {
    std::cerr << "Feed already engaged" << std::endl;
  }

  return 0;
}
#endif
