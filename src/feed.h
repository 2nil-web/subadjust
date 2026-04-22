#ifndef FEED_H
#define FEED_H
#include <filesystem>
#include <string>

class feed
{
  std::filesystem::path feed_dir, file_to_stop_feed, file_to_feed;
  int delay_between_feed;

public:
  feed(std::string feed_name = ".count", std::string feed_stop = ".stop", int _delay_between_feed = 200);
  bool start(); // Start feeding in a thread
  bool stop();
  // Return true if no pending feed
  bool is_being_fed();
};

#endif /* FEED_H */
