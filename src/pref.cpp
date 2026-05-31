
#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "file_features.h"
#include "fonts.h"
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

std::string pref_get_string(const std::string key, const std::string def_val, Fl_Preferences *pref)
{
  if (pref == nullptr)
    pref = &window;
  char *raw = nullptr;
  pref->get(key.c_str(), raw, def_val.c_str());
  std::unique_ptr<char, decltype(&free)> pval(raw, &free);
  return pval ? std::string(pval.get()) : def_val;
}

int pref_get_int(const std::string key, int def_val, Fl_Preferences *pref)
{
  if (pref == nullptr)
    pref = &window;
  int val;
  pref->get(key.c_str(), val, def_val);
  return val;
}

void case_find(Fl_Widget *, void *)
{
  if (case_sensitive_find->value())
    case_sensitive_find->copy_label(_("Case sensitive search"));
  else
    case_sensitive_find->copy_label(_("Case unsensitive search"));
}

void main_window_resize(int x, int y, int w, int h)
{
  // For some reason it is now necessary to call hotspot before calling resize
  // At least to ensure the positionning as designed for this app
  // Don't know if one is enough or
  bool once = true;
  if (once)
  {
    main_window->hotspot(0, 0, 0);
    once = false;
  }

  //  logD("juxtaposing main_window_resize: (", x, ", ", y, ", ", w, ", ", h, ")");
  main_window->resize(x, y, w, h);
}

std::string screen_info()
{
  int x, y, w, h, sc;
  float hr, vr;
  std::stringstream ss;

  sc = Fl::screen_count();
  ss << _("Number of available screens is: ") << sc << ".\n";

  for (int i = 0; i < sc; i++)
  {
    Fl::screen_dpi(hr, vr, i);
    ss << _("\nFor screen") << i << ":\n";
    ss << _S("Horizontal and vertical screen resolution in dots-per-inch") + ": (" << hr << ", " << vr << ").\n";

    Fl::screen_work_area(x, y, w, h, i);
    ss << _S("Bounding box of the work area is") + ": (" << x << ", " << y << ", " << w << ", " << h << ").\n";

    Fl::screen_xywh(x, y, w, h, i);
    ss << _S("Dimensions of the rectangle encompassing the screen") + ": (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  }

  int xm = Fl::x(), ym = Fl::y(), wm = Fl::w(), hm = Fl::h();
  ss << "\n" + _S("Leftmost x and y coordinate, width and height of the main work area") + ": (" << xm << ", " << ym << ", " << wm << ", " << hm << ").\n";

  ss << "\n" + _S("Number of the screen that contains the position") + " (" << xm << ", " << ym << "): " << Fl::screen_num(xm, ym) << ".\n";
  ss << _S("Number for the screen which intersects the most with the rectangle defined by") + " (" << xm << ", " << ym << ", " << wm << ", " << hm << "): " << Fl::screen_num(xm, ym, wm, hm) << ".\n";

  Fl::screen_xywh(x, y, w, h, xm, ym, wm, hm);
  ss << "\n" + _S("Rectangle encompassing the screen having the largest intersection with the rectangle defined by") + " (" << xm << ", " << ym << ", " << wm << ", " << hm << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";

  Fl::screen_work_area(x, y, w, h, xm, ym);
  ss << "\n" + _S("Frame encompassing the screen's work area containing the position") + " (" << xm << ", " << ym << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_xywh(x, y, w, h, xm, ym);
  ss << _S("Frame encompassing the screen containing the position") + " (" << xm << ", " << ym << "): (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_work_area(x, y, w, h);
  ss << _S("Frame encompassing the screen's work area containing the position") + ": (" << x << ", " << y << ", " << w << ", " << h << ").\n";
  Fl::screen_xywh(x, y, w, h);
  ss << _S("Frame encompassing the area of the screen containing the mouse pointer") + ": (" << x << ", " << y << ", " << w << ", " << h << ").\n";

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
  // logD("get_work_area: (", w, ", ", h, ")");
}

void correct_geometry(int &x, int &y, int &w, int &h)
{
  int wmax, hmax;
  screens_work_area(wmax, hmax);

  // logD("Correc - wmax: ", wmax, ", hmax: ", hmax);
  // logD("Correc avant - x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);

  if (w < MIN_W || w > wmax)
  {
    logW("width(", w, ") has been corrected to fit between ", MIN_W, " and ", wmax);
    if (w < MIN_W)
      w = MIN_W;
    else
      w = wmax;
  }

  if (h < MIN_H || h > hmax)
  {
    logW("height(", h, ") has been corrected to fit between ", MIN_H, " and ", hmax);
    if (h < MIN_H)
      h = MIN_H;
    else
      h = hmax;
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

  // logD("Correc apres - x: ", x, ", y: ", y, ", w: ", w, ", h: ", h);
}

// $USERPROFILE/.subadjust_admin/juxtaposing_management || $HOME/.subadjust_admin/juxtaposing_management
const std::filesystem::path placement_dir(admin_file("juxtaposing_management"));
place placement_file(placement_dir);

void pref_trace()
{
  return;
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

  // logD("juxtaposing_manage ", placement_file.number(), " - new_x: ", new_x);
  //  Recompute x origin for app instances after the first one
  if (placement_file.number() > 0 && !force_ruling)
  {
    int work_width, work_height;
    get_my_work_area(work_width, work_height);
    // Cycle within the work area
    new_x = (x + placement_file.number() * w) % work_width;
  }

  main_window_resize(new_x, y, w, h);
  // logD("juxtaposing_manage ", placement_file.number(), " - (", new_x, ", ", y, ", ", w, ", ", h, ")");
  fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
}

void chg_coord(Fl_Int_Input *w, int new_val)
{
  if (Fl::focus() != w && w->changed() == 0)
  {
    // logD("juxtaposing_update - changing cfg x, y, w, h");
    //  w->when(0);
    w->value(new_val);
    w->redraw();
    // w->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);
  }
}

// Si la première instance du programme bouge, alors changement des repères de juxtaposition pour les suivantes
int juxtaposing_update(int)
{
  static int x = -1, y = -1, w = -1, h = -1;
  int new_x = main_window->x_root(), new_y = main_window->y_root(), new_w = main_window->w(), new_h = main_window->h();

  if (new_x == x && new_y == y && new_w == w && new_h == h)
  {
    return 0;
  }

  if (config_dialog->shown())
  {
    // logD("juxtaposing_update - config_dialog shown");
    chg_coord(mw_x, new_x);
    chg_coord(mw_y, new_y);
    chg_coord(mw_w, new_w);
    chg_coord(mw_h, new_h);
  }
  // logD("juxtaposing_update ", placement_file.number(), "     - (", x, ", ", y, ", ", w, ", ", h, ")");
  // logD("juxtaposing_update ", placement_file.number(), " NEW - (", new_x, ", ", new_y, ", ", new_w, ", ", new_h, ")");

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
    // logD("juxtaposing_update ", placement_file.number(), " FSH - (", x, ", ", y, ", ", w, ", ", h, ")");
  }

  return 0;
}

void juxtaposing_end()
{
  // logD("juxtaposing_end - placement_file.number: ", placement_file.number());
  placement_file.leave();

  if (placement_file.is_empty())
    remove_opened(true);
  else
    remove_opened();
}

std::filesystem::path l10n_dir = "";

// Ensure/force that path p is/to an absolute path ending with "locale"
// Return the transformed path in that way and true if it is an existing not empty directory
bool ensure_useful_l10n_dir(std::filesystem::path &p)
{
  p = p.lexically_normal();
  if (p.filename() != "locale")
    p /= "locale";
  p = std::filesystem::absolute(p);
  return (std::filesystem::is_directory(p) && !std::filesystem::is_empty(p));
}

// Traverse a predefined list of path to return the first one that point to an absolute, existing not empty directory ending with "locale"
std::filesystem::path find_locale_dir(std::filesystem::path prog_path, bool anticipate_dir)
{
  // On anticipe la récupération du répertoire l10n, s'il existe
  std::vector<std::filesystem::path> vp = {prog_path, std::filesystem::current_path(), ".", personal_dir(), "/usr/share", "/usr/local/share", my_getenv("locale_dir"), my_getenv("LOCALE_DIR")};

  // Insert the locale dir provided in the config file if wanted and if there is
  if (anticipate_dir)
  {
    std::filesystem::path my_l10n_dir = pref_get_string("locale_dir", "");
    // logD("anticipate l10n_dir: ", my_l10n_dir);
    /*if (ensure_useful_l10n_dir(my_l10n_dir))*/ vp.insert(vp.begin(), my_l10n_dir);
  }
  /*
    for (auto p : vp)
    {
      if (ensure_useful_l10n_dir(p))
        logD(p, " useful_l10n_dir");
      else
        logD(p, " UNuseful_l10n_dir");
    }
  */
  for (auto p : vp)
  {
    if (ensure_useful_l10n_dir(p))
    {
      // logD("Found useful l10n_dir: ", p);
      return p;
    }
  }

  // logD("No l10n_dir found");
  return "";
}

void setup_i18n(std::filesystem::path prog_path, bool anticipate_dir)
{
#if _WIN32
  _configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
  SetThreadLocale(GetUserDefaultLCID());
#else
  setlocale(LC_ALL, "");
  setlocale(LC_CTYPE, "");
  setlocale(LC_MESSAGES, "");
#endif

  std::filesystem::path my_l10n_dir = find_locale_dir(prog_path, anticipate_dir);
  // logD("my_l10n_dir: ", my_l10n_dir);
  bindtextdomain("subadjust", my_l10n_dir.string().c_str());
  bind_textdomain_codeset("subadjust", "UTF-8");
  textdomain("subadjust");
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
  {
    std::string stheme = pref_get_string("theme", "METRO");
    OS::use_theme(stheme.c_str());
  }

  case_sensitive_find->value(pref_get_int("Case sensitive search", 0));
  case_find();

  std::string fv = pref_get_string("find value", R"(\{\\an8\})");
  str_find->value(fv.c_str());

  std::string sf = dup_anti_slash(pref_get_string("find menu", R"(\{\\an8\}|(..:..:..,...))"));
  str_find->add(sf.c_str());

  std::string rv = pref_get_string("replace value", "");
  str_replace->value(rv.c_str());

  std::string sr = dup_anti_slash(pref_get_string("replace menu", R"(|$1)"));
  str_replace->add(sr.c_str());

  l10n_dir = pref_get_string("locale_dir", "");

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

    char *raw = nullptr;
    window.get(key.c_str(), raw, "");
    std::unique_ptr<char, decltype(&free)> pval(raw, &free);

    if (pval && pval.get()[0] != '\0')
    {
      std::string val;
      std::vector<std::string> mv2 = split(std::string(pval.get()), '|');
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

void old_pref_reset()
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

  window.delete_entry("locale_dir");
  window.delete_entry("font name");
  window.delete_entry("font number");
  window.delete_entry("font size");
  std::filesystem::remove_all(placement_dir);
  std::filesystem::remove(already_opened_list); // already_opened_list is defined in file_feature.h
}

void pref_reset()
{
  logT("Resetting prefs");
  myprefs.clear();
  std::filesystem::remove_all(placement_dir);
  std::filesystem::remove(already_opened_list); // already_opened_list is defined in file_feature.h
}

static std::string global_font_name = "";
static int global_font_number = -1, global_font_size = -1;
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

  if (!l10n_dir.empty())
    window.set("locale_dir", l10n_dir.string().c_str());

  if (!global_font_name.empty())
    window.set("font name", global_font_name.c_str());
  if (global_font_size != -1)
    window.set("font size", global_font_size);
  if (global_font_number != -1)
    window.set("font number", global_font_number);

  juxtaposing_end();
}

int old_theme, old_x, old_y, old_w, old_h, work_w, work_h;
std::filesystem::path old_l10n_dir;
std::string old_font_name;
int old_font_number, old_font_size;
std::filesystem::path old_loc_dir = "";

void mw_resize(Fl_Widget *, void *)
{
  int x = std::stoi(mw_x->value()), y = std::stoi(mw_y->value()), w = std::stoi(mw_w->value()), h = std::stoi(mw_h->value());
  if (x >= 0 && y >= 0 && x + w <= work_w && y + h <= work_h)
  {
    // logD("mw_resize: (", x, ", ", y, ", ", w, ", ", h, ")");
    main_window_resize(x, y, w, h);
    // logD("main_window_resize(x, y, w, h): (", x, ", ", y, ", ", w, ", ", h, ")");
  }
}

void unconfig(Fl_Widget *, void *)
{
  OS::use_theme(old_theme);
  main_window_resize(old_x, old_y, old_w, old_h);
  // logD("main_window_resize(old_x, old_y, old_w, h): (", old_x, ", ", old_y, ", ", old_w, ", ", old_h, ")");
  font_redraw(old_font_name, old_font_number, old_font_size);

  if (ensure_useful_l10n_dir(old_l10n_dir) && l10n_dir != old_l10n_dir)
  {
    l10n_dir = old_l10n_dir;
    setup_i18n(l10n_dir, false);
  }
  config_dialog->hide();
}

// Global FLTK callback for drawing all label text
void GlobalDraw(const Fl_Label *o, int X, int Y, int W, int H, Fl_Align a)
{
  // fl_font(o->font, o->size);
  fl_font(global_font_number, global_font_size);
  fl_color((Fl_Color)o->color);
  fl_draw(o->value, X, Y, W, H, a); //, o->image, G_usesymbols);
}

void font_redraw(std::string font_name, int font_number, int font_size)
{
  if (!font_name.empty())
  {
    global_font_name = font_name;
    //    Fl::set_font(FL_HELVETICA, global_font_name.c_str());
  }
  //  else global_font_name = Fl::get_font_name(FL_HELVETICA, nullptr);

  // logD("FONT rdw - name: ", font_name, ", size: ", font_size, ", number: ", font_number);
  global_font_number = font_number;
  global_font_size = font_size;
  Fl::set_labeltype(FL_NORMAL_LABEL, GlobalDraw, nullptr);
  // Fl::set_labeltype(FL_FREE_LABELTYPE, GlobalDraw, nullptr);
  //     Fl::set_font(FL_FREE_FONT, FL_HELVETICA);
  fl_font(font_number, font_size);
  //  Fl::flush();
  main_window->redraw();
  file_content->textfont(font_number);
  file_content->textsize(font_size);
  file_content->redraw();
  config_dialog->redraw();
  //  Fl::flush();
}

void font_manage(Fl_Widget *, void *vfis)
{
  std::vector<sfont_info> fis = *((std::vector<sfont_info> *)vfis);
  int ifn = font_names->value();
  int ifs = font_sizes->value();
  int last_sel_sz;
  if (ifs == 0)
    last_sel_sz = 14;
  else
    last_sel_sz = std::stoi(font_sizes->text(ifs));

  if (ifn > 0 && ifn < (int)fis.size())
  {
    ifn--;
    // logD("FONT mng - name: ", fis[ifn].name, ", size: ", last_sel_sz, ", number: ", ifn, "/", fis.size());

    font_sizes->clear();
    if (fis[ifn].any_size)
    {
      for (int i = 1; i < 61; i++)
      {
        font_sizes->add(std::to_string(i).c_str());
      }

      if (last_sel_sz < 61)
        font_sizes->select(last_sel_sz, true);
      else
        font_sizes->select(14, true);
    }
    else
    {
      int actual_sel_sz = 14;
      int i = 1;
      for (auto n : fis[ifn].sizes)
      {
        font_sizes->add(std::to_string(n).c_str());
        if (n <= last_sel_sz)
        {
          font_sizes->select(i, true);
          actual_sel_sz = n;
          // logD("FONT SIZE NOT ANY: (", n, ")");
        }
        i++;
      }

      if (font_sizes->value() != 0)
        last_sel_sz = actual_sel_sz;
    }

    font_redraw(fis[ifn].name, fis[ifn].number, last_sel_sz);
  }
  // else logD("FONT - Bad name (", ifn, ") or size (", ifs, ") selection");
}

void default_edit(std::filesystem::path p)
{
  std::string editor = my_getenv("EDITOR");
  if (editor.empty())
  {
    editor = "vi";
  }

  std::string edit_cmd("\"" + editor + "\" " + p.string() + " &");
  // logD("edit: ", edit_cmd);
  std::system(edit_cmd.c_str());
}

void view_config(Fl_Widget *, void *)
{
  std::string config_file = std::filesystem::path(pref_filename()).make_preferred().string();
#ifdef _WIN32
  std::wstring stemp = L"\"" + std::wstring(config_file.begin(), config_file.end()) + L"\"";
  if ((INT_PTR)ShellExecute(nullptr, L"edit", stemp.c_str(), nullptr, nullptr, SW_SHOWNORMAL) < 32)
  {
    logD(std::to_string(GetLastError()));
    logD("Try with variable EDITOR, if available");
  }
  else
#endif
    default_edit(config_file);
}

void pref_dialog(Fl_Widget *, void *)
{
  static bool unpopulated_dialog = true;

  if (unpopulated_dialog)
  {
    theme_choice->add("CLASSIC|AERO|METRO|AQUA|GREYBIRD|OCEAN|BLUE|OLIVE|ROSE_GOLD|DARK|BRUSHED_METAL|HIGH_CONTRAST");
    theme_choice->callback(SIMPLE_CB { OS::use_theme(theme_choice->value()); });

    static std::vector<sfont_info> fis;
    get_fonts_info(fis, true, true);
    for (auto fi : fis)
    {
      font_names->add(fi.face_name.c_str());
      // logD("FONT: ", fi);
    }

    font_names->callback(font_manage, (void *)&fis);
    font_sizes->callback(font_manage, (void *)&fis);

    loc_dir_sel->callback(SIMPLE_CB {
      Fl_Native_File_Chooser ld_sel;

      ld_sel.options(Fl_Native_File_Chooser::Option::NEW_FOLDER | Fl_Native_File_Chooser::Option::PREVIEW);
      ld_sel.title(_("Select locale folder"));
      ld_sel.type(Fl_Native_File_Chooser::Type::BROWSE_DIRECTORY);
      switch (ld_sel.show())
      {
      case -1:
        fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);
        fl_alert("%s", ld_sel.errmsg());
        break; // ERROR
      case 1:
        logT("CANCEL\n");
        break; // CANCEL
      default:
        logT("PICKED: %s\n", ld_sel.filename());
        loc_dir->value(ld_sel.filename());
        file_path->value(std::filesystem::absolute(ld_sel.filename()).string().c_str());
        std::filesystem::path new_l10n_dir(ld_sel.filename());

        if (ensure_useful_l10n_dir(new_l10n_dir) && l10n_dir != new_l10n_dir)
        {
          l10n_dir = new_l10n_dir;
          setup_i18n(l10n_dir, false);
        }
      }
    });

    mw_x->callback(mw_resize);
    mw_y->callback(mw_resize);
    mw_w->callback(mw_resize);
    mw_h->callback(mw_resize);

    ok_config->callback(SIMPLE_CB {
      config_dialog->hide();
      old_x = main_window->x_root();
      old_y = main_window->y_root();
      old_w = main_window->w();
      old_h = main_window->h();
      correct_geometry(old_x, old_y, old_w, old_h);
      main_window_resize(old_x, old_y, old_w, old_h);
      logD("main_window_resize(old_x, old_y, old_w, h): (", old_x, ", ", old_y, ", ", old_w, ", ", old_h, ")");
    });

    cancel_config->callback(unconfig);
    config_dialog->callback(unconfig);

    if (placement_file.number() == 0)
      geo_not_saved->hide();
    else
      geo_not_saved->show();

    config_reset->callback(SIMPLE_CB {
      myprefs.clear();
      window.clear();
    });

    config_view->callback(view_config);
    config_reset->callback(SIMPLE_CB {
      pref_reset();
      main_window_resize(DEF_WIN_X, DEF_WIN_Y, DEF_WIN_W, DEF_WIN_H);
      logD("main_window_resize(DEF_WIN_X, DEF_WIN_Y, DEF_WIN_W, DEF_WIN_H): (", DEF_WIN_X, ", ", DEF_WIN_Y, ", ", DEF_WIN_W, ", ", DEF_WIN_H, ")");
      theme_choice->value(2);
      OS::use_theme(theme_choice->value());
      case_sensitive_find->value(0);
      case_sensitive_find->label(_("Case sensitive search"));
    });

    unpopulated_dialog = false;
  }

  if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R))
  {
    config_view->show();
    config_reset->show();
  }
  else
  {
    config_view->hide();
    // config_view->callback(SIMPLE_CB{});
    config_reset->hide();
    // config_reset->callback(SIMPLE_CB{});
  }

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

  theme_choice->value(old_theme);

  old_l10n_dir = l10n_dir;
  loc_dir->value(old_l10n_dir.string().c_str());
  old_font_name = global_font_name;
  old_font_number = global_font_number;
  old_font_size = global_font_size;

  config_dialog->show();
}
