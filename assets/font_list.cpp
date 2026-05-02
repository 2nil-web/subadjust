
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

struct sfont_info
{
  Fl_Font number;
  int attr;
  std::string name, sattr;
  bool any_size;
  std::vector<int> sizes;
};

void get_fonts_info(std::vector<sfont_info> &fis, bool only_normal=false, bool only_any_size=false)
{
  int k = Fl::set_fonts();

  for (int i = 0; i < k; i++)
  {
    sfont_info fi;
    fi.name = Fl::get_font_name((Fl_Font)i, &fi.attr);
    fi.number = i;
    fi.sattr = "";
    if (fi.attr & FL_BOLD)
      fi.sattr += "@b";
    if (fi.attr & FL_ITALIC)
      fi.sattr += "@i";
    fi.sattr += "@.";

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
        for (int j = 0; j < n; j++)
          fi.sizes.push_back(s[j]);
      }
    }

    if ((!only_normal || fi.attr == 0) && (!only_any_size || fi.any_size))
      fis.push_back(fi);
  }

  std::sort(fis.begin(), fis.end(), [](sfont_info a, sfont_info b) {
    if (a.name < b.name)
      return true;
    else if (a.name > b.name)
      return false;
    else if (a.sattr < b.sattr)
      return true;
    return false;
  });
}

int main()
{
  std::vector<sfont_info> fis;
  get_fonts_info(fis);

  std::cout << "\"Font\nnumber\";\"font\nname\";\"font\nattributes\nnumber\";\"font\nattributes\nstring\";\"font\rsizes\"" << std::endl;
  for (auto fi : fis)
  {
    std::cout << fi.number << "; " << fi.name << "; " << fi.attr << "; " << fi.sattr << "; ";

    if (fi.any_size) std::cout << "Any size";
    else if (fi.sizes.size() > 0)
    {
      for (auto size : fi.sizes) std::cout << size << ", ";
    }

    std::cout << std::endl;
  }
  return 0;
}
