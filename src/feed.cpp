
#include <chrono>
#include <thread>
#include <future>
#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

// Get env var
// Return true if exists else empty string
std::string my_getenv(const std::string var, bool msg_if_empty=false)
{
  char *pVal = nullptr;
  std::string sVal = "";
  if (msg_if_empty)
    sVal = "No value found for " + var;

#ifdef _WIN32
  DWORD vl = GetEnvironmentVariableA(var.c_str(), NULL, 0);
  if (vl > 0)
  {
    pVal = new char[vl + 1];
    GetEnvironmentVariableA(var.c_str(), pVal, vl);
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

std::filesystem::path personal_dir() {
  std::filesystem::path var;
#ifdef _WIN32
  var=my_getenv("USERPROFILE");
#else
  var=my_getenv("HOME");
#endif

  return var;
}

class feed {
  bool create_directory_if_possible(std::filesystem::path p)
  {
    if (std::filesystem::is_directory(p))
      return false;
    else
      return std::filesystem::create_directory(p);
  }

  bool create_file_if_possible(std::filesystem::path p)
  {
    if (std::filesystem::exists(p))
      return false;
    else {
      std::ofstream f(p);
      if (f) {
        f.close();
        return true;
      } else return false;
    }
  }

  std::filesystem::path feed_dir, file_to_stop_feed;
  int delay_between_feed;
  bool able_to_start=false;
  public:
  feed(std::string feed_name=".count", std::string feed_stop=".stop", int _delay_between_feed=400)
  {
    feed_dir=personal_dir() / feed_name;
    file_to_stop_feed=feed_dir / feed_stop;
    delay_between_feed=_delay_between_feed;
  }

  bool start()
  {
    create_directory_if_possible(feed_dir);
    if (std::filesystem::is_empty(feed_dir) && create_file_if_possible(feed_dir / "0")) {
      std::thread([this] {
          int feed_index=0;
          std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));

          for(;;) {
            // If file_to_stop_feed exists then only remove actual file to feed
            if (std::filesystem::exists(file_to_stop_feed)) {
              // Wait twice delay_between_feed before removing file to feed
              std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
              // Then remove file_to_feed
              std::filesystem::remove(feed_dir / std::to_string(feed_index));
              break;
            } else { // Else create next and remove previous feed file to feed
              // Create next file to feed
              create_file_if_possible(feed_dir / std::to_string(feed_index + 1));
              // Remove actual file to feed
              std::filesystem::remove(feed_dir / std::to_string(feed_index));
              // Increment feed_index
              feed_index++;
              // And sleep delay_between_feed before next feed
              std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
            }
          }
      }).detach();

      able_to_start=true;
      return true;
    } else return false;
  }

  bool stop()
  {
    // Notice thread to stop
    return create_file_if_possible(file_to_stop_feed);
  }

  // Return true if feed_dir is not empty
  bool is_being_fed()
  {
    return !std::filesystem::is_empty(feed_dir);
  }

  ~feed()
  {
    if (able_to_start) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
      std::filesystem::remove(file_to_stop_feed);
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_between_feed));
      std::filesystem::remove(feed_dir);
    }
  }
};


int main () {
  feed mf;

  if (mf.start()) {
    for (int i=0; i < 10; i++) {
      std::cout << "main thread waiting: " << i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    mf.stop();
  } else {
    std::cerr << "Feed already engaged" << std::endl;
  }

  return 0;
}

