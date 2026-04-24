
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
#include "pref.h"
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

std::string pref_get_string(const std::string key, const std::string def_val, Fl_Preferences pref = window)
{
  char *pval;
  pref.get(key.c_str(), pval, def_val.c_str());
  std::string val = pval;
  free(pval);
  return val;
}

int pref_get_int(const std::string key, int def_val, Fl_Preferences pref = window)
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

  if (w < MIN_W || w > wmax)
  {
    logW("width(", w, ") has been corrected to fit between ", MIN_W, " and ", wmax);
    if (w < MIN_W) w = MIN_W;
    else w = wmax;
  }

  if (h < MIN_H || h > hmax)
  {
    logW("height(", h, ") has been corrected to fit between ", MIN_H, " and ", hmax);
    if (h < MIN_H) h = MIN_H;
    else h = hmax;
  }

  if (x < MIN_X)
  {
    logW("Negative x origin has been corrected to fit into the work area");
    x = MIN_X;
  }

  if (y < MIN_Y)
  {
    logW("Negative y origin has been corrected to fit into the work area");
    y = MIN_Y;
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

void pref_trace()
{
  logT("Trace_prefs ", placement_file.number(), " - Main window geometry: (", main_window->x_root(), ", ", main_window->y_root(), ", ", main_window->w(), ", ", main_window->h(), ')');
  /*
  logT("Trace_prefs - Preferences file name: ", pref_filename());
  logT("Trace_prefs - Main window geometry: (", main_window->x_root(), ", ", main_window->y_root(), ", ", main_window->w(), ", ", main_window->h(), ')');
  logT("Trace_prefs - Theme: ", OS::themes_string());
  logT("Trace_prefs - Find case sensitivity: ", case_sensitive_find->value());
  logT("Trace_prefs - Patterns - find value: ", str_find->value(), ", find menu: ", menu_to_string(str_find));
  logT("Trace_prefs -         replace value: ", str_replace->value(), ", replace menu: ", menu_to_string(str_replace));
  */
}

// Management of the multiple instances main window juxtaposing, juxtaposing rules are :
//   1) Only the first appearing windows will have its configuration parameters saved in the preferences file
//   2) The first appearing window's position will be use as the starting point for all others
//   3) The first appearing window's dimenson will be use for all others
//   4) Juxtaposing is done from left to right on the whole work area
//   5) Horizontally cycle between left most and right most side of the work area
//   6) If one of the window disappears its place will be used by the next appearing one
void juxtaposing_manage(const int x, const int y, const int w, const int h, bool force_ruling = false)
{
  int new_x = x;

  // Recompute x origin for app instances after the first one
  if (placement_file.number() > 0 && !force_ruling)
  {
    int work_width, work_height;
    get_my_work_area(work_width, work_height);
    // Cycle within the work area
    new_x = (x + placement_file.number() * w) % work_width;
  }

  main_window->resize(new_x, y, w, h);
  logD("juxtaposing_manage - placement_file.number: ", placement_file.number(), ", x: ", new_x, ", y: ", y, ", w: ", w, ", h: ", h);
  fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
}

void chg_coord(Fl_Int_Input* w, int new_val)
{
  if (Fl::focus() != w && w->changed() == 0) {
    logD("juxtaposing_update - changing cfg x, y, w, h");
    //w->when(0);
    w->value(new_val);
    w->redraw();
    //w->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
  }
}

// Si la première instance du programme bouge, alors changement des repères de juxtaposition pour les suivantes
int juxtaposing_update(int)
{
  static int x = -1, y = -1, w = -1, h = -1;
  int new_x = main_window->x_root(), new_y = main_window->y_root(), new_w = main_window->w(), new_h = main_window->h();

  if (new_x == x && new_y == y && new_w == w && new_h == h) return 0;

  if (config->shown()) {
    chg_coord(mw_x, new_x);
    chg_coord(mw_y, new_y);
    chg_coord(mw_w, new_w);
    chg_coord(mw_h, new_h);
  }

  if (placement_file.number() == 0)
  {
    if (x != new_x)
    {
      x = new_x;
      window.set("xpos", x);
    }

    if (y != new_y)
    {
      y = new_y;
      window.set("ypos", y);
    }

    if (w != new_w)
    {
      w = new_w;
      window.set("width", w);
    }

    if (h != new_h)
    {
      h = new_h;
      window.set("height", h);
    }

    // Force la mise à jour des données de préférence
    window.flush();
    logD("juxtaposing_update ", placement_file.number(), " - x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);
  }

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

void pref_get(int x, int y, int w, int h)
{
  //  remove_cr_in_log(false); logI(screen_info_fr()); remove_cr_in_log();
  logT("Restoring prefs");
  if (x == -1)
    x = pref_get_int("xpos", DEF_WIN_X);
  if (y == -1)
    y = pref_get_int("ypos", DEF_WIN_Y);
  if (w == -1)
    w = pref_get_int("width", DEF_WIN_W);
  if (h == -1)
    h = pref_get_int("height", DEF_WIN_H);

  correct_geometry(x, y, w, h);
  juxtaposing_manage(x, y, w, h);
  Fl::add_handler(juxtaposing_update);
  std::at_quick_exit(juxtaposing_end);
  std::atexit(juxtaposing_end);

  extern std::string theme;
  if (theme == "")
    OS::use_theme(pref_get_string("theme", "METRO").c_str());

  case_sensitive_find->value(pref_get_int("case", 0));
  // case_find();

  str_find->value(pref_get_string("find value", R"(\{\\an8\})").c_str());
  str_find->add(dup_anti_slash(pref_get_string("find menu", R"(\{\\an8\}|(..:..:..,...))")).c_str());

  str_replace->value(pref_get_string("replace value", "").c_str());
  str_replace->add(dup_anti_slash(pref_get_string("replace menu", R"(|$1)")).c_str());

  pref_trace();
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

void pref_reset()
{
  logT("Resetting prefs");
  window.set("xpos", DEF_WIN_X);
  window.set("ypos", DEF_WIN_Y);
  window.set("width", DEF_WIN_W);
  window.set("height", DEF_WIN_H);
  window.set("theme", "METRO");
  window.set("case", 0);
  window.set("find value", "");
  window.set("replace value", "");

  window.set("find menu", "\\{\\\\an8\\}|(..:..:..,...)|<font|<font color=\"#......\">");
  window.set("replace menu", "||$1");
  window.flush();

  std::filesystem::remove_all(placement_dir);
  const std::filesystem::path already_opened_list;
  std::filesystem::remove(already_opened_list);
}

void pref_set()
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

  pref_trace();

  juxtaposing_end();
}

int old_theme, old_x, old_y, old_w, old_h, work_w, work_h;

void mw_resize(Fl_Widget *, void *)
{
  int x = std::stoi(mw_x->value()), y = std::stoi(mw_y->value()), w = std::stoi(mw_w->value()), h = std::stoi(mw_h->value());
  if (x >= 0 && y >= 0 && x + w <= work_w && y + h <= work_h) {
    logD("mw_resize: (", x, ", ", y, ", ", w, ", ", h, ")");
    main_window->resize(x, y, w, h);
  }
}

void unconfig(Fl_Widget*, void*)
{
    OS::use_theme(old_theme);
    main_window->resize(old_x, old_y, old_w, old_h);
    config->hide();
}

void pref_dialog()
{
  old_theme = OS::current_theme();
  old_x = main_window->x_root();
  old_y = main_window->y_root();
  old_w = main_window->w();
  old_h = main_window->h();
  get_my_work_area(work_w, work_h);

  theme_choice->value(old_theme);
  mw_x->value(old_x);
  mw_y->value(old_y);
  mw_w->value(old_w);
  mw_h->value(old_h);

  theme_choice->add("CLASSIC|AERO|METRO|AQUA|GREYBIRD|OCEAN|BLUE|OLIVE|ROSE_GOLD|DARK|BRUSHED_METAL|HIGH_CONTRAST");
  theme_choice->value(old_theme);
  theme_choice->callback(SIMPLE_CB { OS::use_theme(theme_choice->value()); });

  mw_x->callback(mw_resize);
  mw_y->callback(mw_resize);
  mw_w->callback(mw_resize);
  mw_h->callback(mw_resize);

  ok_config->callback(SIMPLE_CB {
    config->hide();
    old_x = main_window->x_root();
    old_y = main_window->y_root();
    old_w = main_window->w();
    old_h = main_window->h();
    correct_geometry(old_x, old_y, old_w, old_h);
    main_window->resize(old_x, old_y, old_w, old_h);
  });

  cancel_config->callback(unconfig);
  config->callback(unconfig);

  if (placement_file.number() == 0)
    geo_not_saved->hide();
  else
    geo_not_saved->show();

  config->show();
}
