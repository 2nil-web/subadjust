
#include "fonts.h"

Fl_Font ensure_only_one_call_to_set_fonts()
{
  static bool not_called=true;
  static Fl_Font num_fonts=0;
  static size_t ncall=0;


  if (not_called) {
    std::cout << "FONT - ensure_only_one_call_to_set_fonts. This line should appears only once" << std::endl;
    num_fonts=Fl::set_fonts();
    not_called=false;
  }

  std::cout << "FONT - ensure_only_one_call_to_set_fonts, call number " << ncall++ << ", there are " << num_fonts << " fonts available" << std::endl;

  return num_fonts;
}

// Nouveau : cache des noms capturés immédiatement après set_fonts()
const std::vector<std::string>& get_font_names_cache()
{
    static std::vector<std::string> cache;
    static bool built = false;

    if (!built) {
        int k = ensure_only_one_call_to_set_fonts();
        cache.reserve(k);
        for (int i = 0; i < k; i++) {
            const char *name = Fl::get_font_name((Fl_Font)i, nullptr);
            cache.push_back(name ? name : "");
        }
        built = true;
    }
    return cache;
}

void get_fonts_info(std::vector<sfont_info> &fis, bool only_normal, bool only_any_size)
{
  const auto& names = get_font_names_cache();
  int k = (int)names.size();

  for (int i = 0; i < k; i++)
  {
    sfont_info fi;
    fi.name = names[i];
    Fl::get_font_name((Fl_Font)i, &fi.attr);
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
    else
      fi.any_size = true;

    fi.number = i;
    fi.sattr = "";
    if (fi.attr & FL_BOLD)
      fi.sattr += "@b";
    if (fi.attr & FL_ITALIC)
      fi.sattr += "@i";
    fi.sattr += "@.";

    if ((!only_normal || fi.attr == 0) && (!only_any_size || fi.any_size))
    {
      // logD("FONT: ", fi.number, "; ", fi.name, "; ", fi.attr, "; ", fi.sattr);
      fis.push_back(fi);
    }
  }

  std::sort(fis.begin(), fis.end(), [](sfont_info &a, sfont_info &b) {
    if (a.name < b.name)
      return true;
    else if (a.name > b.name)
      return false;
    else if (a.sattr < b.sattr)
      return true;
    return false;
  });
}

std::string fonts_info_string(bool only_normal, bool only_any_size)
{
  std::vector<sfont_info> fis;
  get_fonts_info(fis, only_normal, only_any_size);

  std::stringstream ss;
  for (auto fi : fis)
  {
    ss << fi.number << "; " << "FONT: " << fi.name << "; " << fi.attr << "; " << fi.sattr << "; " << fi.sizes.size() << std::endl;
  }

  return ss.str();
}


static Fl_Double_Window *font_sel_win=nullptr;
Fl_Tile *tile;

class FontDisplay : public Fl_Widget
{
  void draw() FL_OVERRIDE;

public:
  int font, size;
  FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char *L = 0) : Fl_Widget(X, Y, W, H, L)
  {
    box(B);
    font = 0;
    size = 14;
  }
};

void FontDisplay::draw()
{
  draw_box();
  fl_font((Fl_Font)font, size);
  fl_color(FL_BLACK);
  fl_draw(label(), x() + 3, y() + 3, w() - 6, h() - 6, align());
}

FontDisplay *textobj;

Fl_Hold_Browser *fontobj, *sizeobj;

int **sizes;
int *numsizes;
int pickedsize = 14;

void font_cb(Fl_Widget *, long)
{
  int fn = fontobj->value();
  if (!fn)
    return;
  std::cout << "SEL FONT:" << fontobj->value() << ", " << fontobj->text(fontobj->value()) << std::endl;
  fn--;
  textobj->font = fn;
  sizeobj->clear();
  int n = numsizes[fn];
  int *s = sizes[fn];
  if (!n)
  {
    // no sizes
  }
  else if (s[0] == 0)
  {
    // many sizes;
    int j = 1;
    for (int i = 1; i < 64 || i < s[n - 1]; i++)
    {
      char buf[20];
      if (j < n && i == s[j])
      {
        snprintf(buf, 20, "@b%d", i);
        j++;
      }
      else
        snprintf(buf, 20, "%d", i);
      sizeobj->add(buf);
    }
    sizeobj->value(pickedsize);
  }
  else
  {
    // some sizes
    int w = 0;
    for (int i = 0; i < n; i++)
    {
      if (s[i] <= pickedsize)
        w = i;
      char buf[20];
      snprintf(buf, 20, "@b%d", s[i]);
      sizeobj->add(buf);
    }
    sizeobj->value(w + 1);
  }
  textobj->redraw();
}

void size_cb(Fl_Widget *, long)
{
  int i = sizeobj->value();
  if (!i)
    return;
  const char *c = sizeobj->text(i);
  while (*c < '0' || *c > '9')
    c++;
  pickedsize = atoi(c);
  textobj->size = pickedsize;
  textobj->redraw();
}

void create_font_selector()
{
  font_sel_win = new Fl_Double_Window(550, 200);
  //font_sel_win->clear_border();
  Fl_Group *fontgroup = new Fl_Group(0, 0, 550, 185);
  fontgroup->box(FL_FLAT_BOX);
  fontobj = new Fl_Hold_Browser(10, 5, 390, 170);
  fontobj->box(FL_FRAME_BOX);
  fontobj->color(53, 3);
  fontobj->callback(font_cb);
  sizeobj = new Fl_Hold_Browser(410, 5, 130, 170);
  sizeobj->box(FL_FRAME_BOX);
  sizeobj->color(53, 3);
  sizeobj->callback(size_cb);
  fontgroup->resizable(fontobj);
  fontgroup->end();

  font_sel_win->end();

  auto& names = get_font_names_cache();
  int k = (int)names.size();
  sizes = new int *[k];
  numsizes = new int[k];
  for (int i = 0; i < k; i++)
  {
    int t;
	auto sname = names[i];
    sname[sname.size()] = '\0';
    const char *name = sname.c_str();
    Fl::get_font_name((Fl_Font)i, &t);
    char buffer[256];

    if (t)
    {
      char *p = buffer;
      if (t & FL_BOLD)
      {
        *p++ = '@';
        *p++ = 'b';
      }
      if (t & FL_ITALIC)
      {
        *p++ = '@';
        *p++ = 'i';
      }
      *p++ = '@';
      *p++ = '.';
      my_strcpy(p, name);
      name = buffer;
    }

    fontobj->add(name);
    std::cout << name;
    int *s;
    int n = Fl::get_font_sizes((Fl_Font)i, s);
    numsizes[i] = n;
    if (n)
    {
      std::cout << "; ";
      sizes[i] = new int[n];
      for (int j = 0; j < n; j++)
      {
        sizes[i][j] = s[j];
        std::cout << s[j] << ", ";
      }
    }
    std::cout << std::endl;
  }
  fontobj->value(1);
  font_cb(fontobj, 0);
}

void show_font_selector()
{
   if (font_sel_win == nullptr)
       create_font_selector();
   font_sel_win->show();
}

