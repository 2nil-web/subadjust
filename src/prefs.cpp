
#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "file_features.h"
#include "log.h"
#include "place.h"
#include "prefs.h"
#include "subadjust_ui.h"
#include "themes.h"
#include "utils.h"

Fl_Preferences myprefs(Fl_Preferences::USER_L, "dplalanne.fr", "subadjust");
Fl_Preferences window(myprefs, "window");
std::string find_patterns;
std::string replace_patterns;

std::string menu_to_string(Fl_Input_Choice *ic)
{
  std::string smenu = "";

  if (ic)
  {
    const Fl_Menu_Item *mis = ic->menu();
    for (int i = 0; i < mis->size() - 1; i++)
    {
      smenu += std::string("|") + std::string(mis->next(i)->label());
    }
  }

  return smenu;
}

const std::string pref_filename()
{
  char pref_fn[FL_PATH_MAX];
  myprefs.filename(pref_fn, FL_PATH_MAX);
  std::string ret(pref_fn);
  return ret;
}

void trace_prefs()
{
  logT("Preferences file name: ", pref_filename());
  logT("Main window geometry: (", main_window->x_root(), ", ", main_window->y_root(), ", ", main_window->w(), ", ", main_window->h(), ')');
  logT("Theme: ", OS::themes_string());
  logT("Find case sensitivity: ", case_sensitive_find->value());
  logT("Patterns - find value: ", str_find->value(), ", find menu: ", menu_to_string(str_find));
  logT("        replace value: ", str_replace->value(), ", replace menu: ", menu_to_string(str_replace));
}

std::string pref_get_string(Fl_Preferences &pref, const std::string key, const std::string def_val)
{
  char *pval;
  pref.get(key.c_str(), pval, def_val.c_str());
  std::string val = pval;
  free(pval);
  return val;
}

int pref_get_int(Fl_Preferences &pref, const std::string key, int def_val)
{
  int val;
  pref.get(key.c_str(), val, def_val);
  return val;
}

void case_find(Fl_Widget *w, void *v)
{
  (void)w;
  (void)v;

  if (case_sensitive_find->value())
    case_sensitive_find->label("Case");
  else
    case_sensitive_find->label("Nocase");
}

std::string screen_info()
{
  int x, y, w, h, sc;
  float hr, vr;
  std::stringstream ss;

  sc = Fl::screen_count();
  ss << "Number of available screens is: " << sc << ".\n";

  for (int i = 0; i < sc; i++)
  {
    Fl::screen_dpi(hr, vr, i);
    ss << "\nFor screen" << i << ":\n";
    ss << "Horizontal and vertical screen resolution in dots-per-inch are: (" << hr << ", " << vr << ").\n";

    Fl::screen_work_area(x, y, w, h, i);
    ss << "Bounding box of the work area is: (" << x << ", " << y << ", " << w << ", " << h << ").\n";

    Fl::screen_xywh(x, y, w, h, i);
    ss << "Screen bounding rect is: (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  }

  int xm = Fl::x(), ym = Fl::y(), wm = Fl::w(), hm = Fl::h();
  ss << "\nLeftmost x and y coordinate and width and height of the main screen work area: (" << xm << ", " << ym << ", " << wm << ", " << hm << ").\n";

  ss << "\nScreen number of the screen that contains the position (" << xm << ", " << ym << "): " << Fl::screen_num(xm, ym) << ".\n";
  ss << "Screen number for the screen which intersects the most with the rectangle defined by (" << xm << ", " << ym << ", " << wm << ", " << hm << "): " << Fl::screen_num(xm, ym, wm, hm) << ".\n";

  Fl::screen_xywh(x, y, w, h, xm, ym, wm, hm);
  ss << "\nScreen bounding rect for the screen which intersects the most with the rectangle defined by (" << xm << ", " << ym << ", " << wm << ", " << hm << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";

  Fl::screen_work_area(x, y, w, h, xm, ym);
  ss << "\nBounding box of the work area of a screen that contains the screen position (" << xm << ", " << ym << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_xywh(x, y, w, h, xm, ym);
  ss << "Bounding box of the screen that contains the screen position (" << xm << ", " << ym << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_work_area(x, y, w, h);
  ss << "Bounding box of the work area of the screen that contains the mouse pointer: (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_xywh(x, y, w, h);
  ss << "Bounding box of the screen that contains the mouse pointer: (" << x << ", " << y << ", " << w << ", " << h << ").\n";

  return ss.str();
}

std::string screen_info_fr()
{
  int x, y, w, h, sc;
  float hr, vr;
  std::stringstream ss;

  sc = Fl::screen_count();
  ss << "Nombre d'écrans disponibles;" << sc << "\n";

  for (int i = 0; i < sc; i++)
  {
    Fl::screen_dpi(hr, vr, i);
    ss << "\nPour l'écran " << i + 1 << ";\n";
    ss << "Résolution horizontale et verticale, en points par pouce;(" << hr << ", " << vr << ")\n";

    Fl::screen_work_area(x, y, w, h, i);
    ss << "Boîte englobante de la zone de travail;(" << x << ", " << y << ", " << w << ", " << h << ")\n";

    Fl::screen_xywh(x, y, w, h, i);
    ss << "Dimensions du rectangle englobant l'écran;(" << x << ", " << y << ", " << w << ", " << h << ")\n";
  }

  int xm = Fl::x(), ym = Fl::y(), wm = Fl::w(), hm = Fl::h();
  ss << "\nCoordonnées x et y les plus à gauche, largeur et hauteur de la zone de travail principale;(" << xm << ", " << ym << ", " << wm << ", " << hm << ")\n";

  ss << "\nNuméro de l'écran contenant la position (" << xm << ", " << ym << ");" << Fl::screen_num(xm, ym) + 1 << "\n";
  ss << "Numéro de l'écran ayant la plus grande intersection avec le rectangle défini par (" << xm << ", " << ym << ", " << wm << ", " << hm << ");" << Fl::screen_num(xm, ym, wm, hm) + 1 << "\n";

  Fl::screen_xywh(x, y, w, h, xm, ym, wm, hm);
  ss << "\nRectangle englobant l'écran ayant la plus grande intersection avec le rectangle défini par (" << xm << ", " << ym << ", " << wm << ", " << hm << ");(" << x << ", " << y << ", " << w << ", " << h << ")\n";

  Fl::screen_work_area(x, y, w, h, xm, ym);
  ss << "\nCadre englobant la zone de travail de l'écran contenant la position (" << xm << ", " << ym << ");(" << x << ", " << y << ", " << w << ", " << h << ")\n";
  Fl::screen_xywh(x, y, w, h, xm, ym);
  ss << "Cadre englobant l'écran contenant la position (" << xm << ", " << ym << ");(" << x << ", " << y << ", " << w << ", " << h << ")\n";
  Fl::screen_work_area(x, y, w, h);

  ss << "\nCadre englobant la zone de travail de l'écran contenant le pointeur de la souris;(" << x << ", " << y << ", " << w << ", " << h << ")\n";
  Fl::screen_xywh(x, y, w, h);
  ss << "Cadre englobant la zone de l'écran contenant le pointeur de la souris;(" << x << ", " << y << ", " << w << ", " << h << ")\n";

  return ss.str();
}

// Compute the screens_work_area size which is supposed to be the biggest sum of x+w and y+h among all the available screens
void screens_work_area(int &_w, int &_h)
{
  int sc = Fl::screen_count(), wmax = 0, hmax = 0, x, w, y, h;

  for (int i = 0; i < sc; i++)
  {
    Fl::screen_work_area(x, y, w, h, i);
    if (wmax < x + w)
      wmax = x + w;
    if (hmax < y + h)
      hmax = y + h;
  }

  _w = wmax;
  _h = hmax;
  logI("screens_work_area: w: ", wmax, ", h: ", hmax);
}

//// I suppose that the difference between screen_work_area and screen_xywh is the(s) task bar(s), but anyway I'm just using screen_work_area.

// Compute the screens_wh which is supposed to be the biggest sum of x+w and y+h among all the available screens
void screens_wh(int &_w, int &_h)
{
  int sc = Fl::screen_count(), wmax = 0, hmax = 0, x, w, y, h;

  for (int i = 0; i < sc; i++)
  {
    Fl::screen_xywh(x, y, w, h, i);
    if (wmax < x + w)
      wmax = x + w;
    if (hmax < y + h)
      hmax = y + h;
  }

  _w = wmax;
  _h = hmax;
  logI("screens_wh: w: ", wmax, ", h: ", hmax);
}

void get_my_work_area(int &w, int &h, bool all_screens = true)
{
  if (all_screens)
  {
    screens_work_area(w, h);
  }
  else
  {
    int sc = Fl::screen_num(main_window->x_root(), main_window->y_root()), x, y;
    Fl::screen_work_area(x, y, w, h, sc);
  }
  logD("get_work_area: (", w, ", ", h, ")");
}

void correct_geometry(int &x, int &y, int &w, int &h)
{
  int wmax, hmax;
  screens_work_area(wmax, hmax);

  logD("Correc - wmax: ", wmax, ", hmax: ", hmax);
  logD("Correc avant - x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);

  if (w < 384 || w > wmax)
  {
    logW("width(", w, ") greater than ", wmax, " has been corrected to fit into the work area");
    w = 384;
  }

  if (h < 500 || h > hmax)
  {
    logW("height(", h, ") greater than ", hmax, " has been corrected to fit into the work area");
    h = 500;
  }

  if (x < 0)
  {
    logW("Negative x origin has been corrected to fit into the work area");
    x = 8;
  }

  if (y < 0)
  {
    logW("Negative y origin has been corrected to fit into the work area");
    y = 8;
  }

  if (x > wmax)
  {
    logW("x(", x, ") origin greater than ", wmax, " has been corrected to fit into the work area");
    x = wmax - w;
  }

  if (y > hmax)
  {
    logW("y(", y, ") origin greater than ", hmax, " has been corrected to fit into the work area");
    y = hmax - h;
  }

  logD("Correc apres - x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);
}

// $USERPROFILE/.subadjust_admin/juxtaposing_management || $HOME/.subadjust_admin/juxtaposing_management
const std::filesystem::path placement_dir(admin_file("juxtaposing_management"));
place placement_file(placement_dir);

// Management of the multiple instances main window juxtaposing, juxtaposing rules are :
//   1) Only the first appearing windows will have its configuration parameters saved in the preferences file
//   2) The first appearing window's position will be use as the starting point for all others
//   3) The first appearing window's dimenson will be use for all others
//   4) Juxtaposing is done from left to right on the whole work area
//   5) Once reached the rightest side of the work area then go back to leftest side
//   6) If one of the window disappears its place will be used by the next appearing one
void juxtaposing_manage(const int x, const int y, const int w, const int h, bool force_ruling = false)
{
  int new_x = x;

  // Recompute x origin for app instances after the first one
  if (placement_file.number() > 0 && !force_ruling)
  {
    int work_width, work_height;
    get_my_work_area(work_width, work_height);
    new_x = x + placement_file.number() * w;
    // Cycle between work area origin +30 and width -30
    if (new_x > work_width - 30)
      new_x = 30;
  }

  main_window->resize(new_x, y, w, h);
  logD("juxtaposing_manage - placement_file.number: ", placement_file.number(), ", x: ", new_x, ", y: ", y, ", w: ", w, ", h: ", h);
  fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
}

// Si la première instance du programme bouge, alors changement des repères de juxtaposition pour les suivantes
int juxtaposing_update(int)
{
  static int x = main_window->x_root(), y = main_window->y_root(), w = main_window->w(), h = main_window->h();

  if (placement_file.number() == 0)
  {

    if (x != main_window->x_root())
    {
      x = main_window->x_root();
      window.set("xpos", x);
    }

    if (y != main_window->y_root())
    {
      y = main_window->y_root();
      window.set("ypos", y);
    }

    if (w != main_window->w())
    {
      w = main_window->w();
      window.set("width", w);
    }

    if (h != main_window->h())
    {
      h = main_window->h();
      window.set("height", h);
    }
  }

  logD("juxtaposing_update - placement_file.number: ", placement_file.number(), ", x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);
  return 0;
}

void juxtaposing_end()
{
  logD("juxtaposing_end - placement_file.number: ", placement_file.number());
  placement_file.leave();

  if (placement_file.is_empty())
    remove_opened(true);
  else
    remove_opened();
}

void get_prefs(int x, int y, int w, int h)
{
  //  remove_cr_in_log(false); logI(screen_info_fr()); remove_cr_in_log();
  logT("Restoring prefs");
  if (x == -1)
    x = pref_get_int(window, "xpos", 8);
  if (y == -1)
    y = pref_get_int(window, "ypos", 30);
  if (w == -1)
    w = pref_get_int(window, "width", 384);
  if (h == -1)
    h = pref_get_int(window, "height", 1000);

  correct_geometry(x, y, w, h);
  juxtaposing_manage(x, y, w, h);
  Fl::add_handler(juxtaposing_update);
  std::at_quick_exit(juxtaposing_end);
  std::atexit(juxtaposing_end);

  extern std::string theme;
  if (theme == "")
    OS::use_theme(pref_get_string(window, "theme", "METRO").c_str());

  case_sensitive_find->value(pref_get_int(window, "case", 0));
  // case_find();

  str_find->value(pref_get_string(window, "find value", R"(\{\\an8\})").c_str());
  str_find->add(dup_anti_slash(pref_get_string(window, "find menu", R"(\{\\an8\}|(..:..:..,...))")).c_str());

  str_replace->value(pref_get_string(window, "replace value", "").c_str());
  str_replace->add(dup_anti_slash(pref_get_string(window, "replace menu", R"(|$1)")).c_str());

  trace_prefs();
}

// Merge menu in memory to its counterpart in the pref file
std::string merge_menu(const std::string key, Fl_Input_Choice *ic, std::string _val)
{
  std::string smenu = "";

  if (ic)
  {
    std::vector<std::string> mv;
    const Fl_Menu_Item *mis = ic->menu();
    for (int i = 0; i < mis->size() - 1; i++)
      mv.push_back(std::string(mis->next(i)->label()));

    mv.push_back(_val);

    char *pval;
    window.get(key.c_str(), pval, "");

    if (pval && pval[0] != '\0')
    {
      std::string val;
      std::vector<std::string> mv2 = split(std::string(pval), '|');
      free(pval);
      mv.insert(mv.end(), mv2.begin(), mv2.end());
      // Sort the vector
      sort(mv.begin(), mv.end());
      // Group unique elements together
      auto it = unique(mv.begin(), mv.end());
      // Erase duplicates
      mv.erase(it, mv.end());
    }

    for (auto m : mv)
      smenu += std::string("|") + m;

    window.set(key.c_str(), smenu.c_str());
  }

  return smenu;
}

void reset_prefs()
{
  logT("Resetting prefs");
  window.set("xpos", 40);
  window.set("ypos", 20);
  window.set("width", 384);
  window.set("height", 600);
  window.set("theme", "METRO");
  window.set("case", 0);
  window.set("find value", "");
  window.set("replace value", "");

  window.set("find menu", "\\{\\\\an8\\}|(..:..:..,...)|<font|<font color=\"#......\">");
  window.set("replace menu", "||$1");

  std::filesystem::remove_all(placement_dir);
  const std::filesystem::path already_opened_list;
  std::filesystem::remove(already_opened_list);
}

void set_prefs()
{
  // On ne sauvegarde la geometrie que de la première instance
  if (placement_file.number() == 0)
  {
    logT("Saving prefs");
    window.set("xpos", main_window->x_root());
    window.set("ypos", main_window->y_root());
    window.set("width", main_window->w());
    window.set("height", main_window->h());
  }

  window.set("theme", OS::current_theme_string().c_str());
  window.set("case", case_sensitive_find->value());
  window.set("find value", str_find->value());
  window.set("replace value", str_replace->value());

  // Always merge menus
  merge_menu("find menu", str_find, str_find->value());
  merge_menu("replace menu", str_replace, str_replace->value());

  trace_prefs();

  juxtaposing_end();
}
