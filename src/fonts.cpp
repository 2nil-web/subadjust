
#include "fonts.h"

bool operator==(const sfont_info &lhs, const sfont_info &rhs)
{
  return lhs.face_name == rhs.face_name && lhs.attr == rhs.attr;
}

bool operator>(const sfont_info &lhs, const sfont_info &rhs)
{
  if (lhs.face_name == rhs.face_name)
    return lhs.sattr > rhs.sattr;
  else
    return lhs.face_name > rhs.face_name;
}

bool operator<(const sfont_info &lhs, const sfont_info &rhs)
{
  if (lhs.face_name == rhs.face_name)
    return lhs.sattr < rhs.sattr;
  else
    return lhs.face_name < rhs.face_name;
}

std::ostream &operator<<(std::ostream &os, const sfont_info &fi)
{
  os << "number: " << fi.number << ", "
     << "attr: " << fi.attr << ", "
     << "face_name: " << fi.face_name << ", "
     << "name: " << fi.name << ", "
     << "any_size: " << fi.any_size << ", "
     << "sizes.size(): " << fi.sizes.size() << ", ";

  //    for (auto c:fi.face_name) os << '[' << (int)c << ']';
  return os;
}

Fl_Font ensure_only_one_call_to_set_fonts()
{
  static bool not_called = true;
  static Fl_Font num_fonts = 0;
  static size_t ncall = 0;

  if (not_called)
  {
    //logD("FONT - ensure_only_one_call_to_set_fonts. This line should appears only once");
    num_fonts = Fl::set_fonts();
    not_called = false;
  }

  //logD("FONT - ensure_only_one_call_to_set_fonts, call number ", ncall++, ", there are ", num_fonts, " fonts available");

  return num_fonts;
}

// Cache des noms capturés immédiatement après set_fonts()
const std::vector<std::string> &get_font_names_cache()
{
  static std::vector<std::string> cache;
  static bool built = false;

  if (!built)
  {
    int k = ensure_only_one_call_to_set_fonts();
    cache.reserve(k);
    for (int i = 0; i < k; i++)
    {
      const char *name = Fl::get_font_name((Fl_Font)i, nullptr);
      cache.push_back(name ? name : "");
    }
    built = true;
  }
  return cache;
}

void get_fonts_info(std::vector<sfont_info> &fis, bool no_attr, bool only_any_size)
{
  const auto &names = get_font_names_cache();
  int k = (int)names.size();

  for (int i = 0; i < k; i++)
  {
    sfont_info fi;
    Fl::get_font_name((Fl_Font)i, &fi.attr);
    fi.face_name = fi.name = names[i];
    // Remove @ from name
    fi.face_name.erase(std::remove(fi.face_name.begin(), fi.face_name.end(), '@'), fi.face_name.end());
    int *s;
    int n = Fl::get_font_sizes((Fl_Font)i, s);
    fi.sizes.clear();
    if (n > 0)
    {
      if (s[0] == 0)
        fi.any_size = true;
      else
      {
        fi.any_size = false;
        fi.face_name = "~" + fi.face_name;
        for (int j = 0; j < n; j++)
          fi.sizes.push_back(s[j]);
      }
    }
    else
      fi.any_size = true;

    fi.number = i;
    fi.sattr = "";
    if (fi.attr & FL_BOLD)
      fi.sattr += "@b";
    if (fi.attr & FL_ITALIC)
      fi.sattr += "@i";
    fi.sattr += "@.";

    if ((fi.attr == 0 || !no_attr) && (!only_any_size || fi.any_size))
    {
      // logD("FONT: ", fi.number, "; ", fi.face_name, "; ", fi.attr, "; ", fi.sattr);
      fis.push_back(fi);
    }
  }

  // Sort fonts
  std::sort(fis.begin(), fis.end());
  // Remove duplicates
  fis.erase(std::unique(fis.begin(), fis.end()), fis.end());
}

std::string fonts_info_string(bool no_attr, bool only_any_size)
{
  std::vector<sfont_info> fis;
  get_fonts_info(fis, no_attr, only_any_size);

  std::stringstream ss;
  for (auto fi : fis)
  {
    ss << "FONT: " << fi << std::endl;
    // ss << fi.number << "; " << "FONT: " << fi.face_name << "; " << fi.attr << "; " << fi.sattr << "; " << fi.sizes.size() << std::endl;
  }

  return ss.str();
}
