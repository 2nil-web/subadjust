#ifndef FILE_FEATURES_H
#define FILE_FEATURES_H

#include <FL/Fl_Text_Buffer.H>
#include <filesystem>
#include <string>

#include "subs.h"

extern Fl_Text_Buffer txt_buf;
extern bool file_is_modified;
extern cSub csub;

void set_file_state(bool modified);
void file_modified(int pos, int nInserted, int nDeleted, int nRestyled, const char * /*deletedText*/, void *);
bool file_read(const char *filename);
bool file_read(std::string filename = "");
bool file_read(std::filesystem::path filename);
bool file_write(std::string filename);

void pre_process(int, int, int, int, int);
void gui_display(bool, bool test_already_opened = true);
void cui_display(bool, std::ostream &);

bool file_handler(bool for_read = true);
bool srt_save();
void remove_opened(std::filesystem::path);
void remove_opened(bool all = false);

#endif /* FILE_FEATURES_H */
