#ifndef FILE_FEATURES_H
#define FILE_FEATURES_H

#include <FL/Fl_Text_Buffer.H>
#include <filesystem>
#include <string>

#include "subs.h"
#include "utils.h"

enum eHandlingType
{
  WRITE = 0,
  READ = 1,
  SYNC = 2
};

extern Fl_Text_Buffer txt_buf;
extern bool file_is_modified;
extern cSub csub;
extern const std::filesystem::path already_opened_list;

void set_file_state(bool modified);
void file_modified(int pos, int nInserted, int nDeleted, int nRestyled, const char * /*deletedText*/, void *);
bool file_read(const char *filename);
bool file_read(std::string filename = "");
bool file_read(std::filesystem::path filename);
bool file_write(std::filesystem::path filename);

void pre_process(int, int, int, int, int);
void gui_display(bool, bool test_already_opened = true);
void cui_display(bool, std::ostream &);
void cui_display(bool, std::filesystem::path);

bool file_handler(eHandlingType ht = eHandlingType::WRITE);
bool save();
void remove_opened(std::filesystem::path);
void remove_opened(bool all = false);
void blink_title(std::string);

#endif /* FILE_FEATURES_H */
