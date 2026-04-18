
#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "file_features.h"
#include "log.h"
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

const std::filesystem::path instance_name(std::filesystem::temp_directory_path() / "subadjust_juxtaposing_management"); // <=> $LOCALAPPDATA/Temp/subadjust_juxtaposing_management || /tmp/subadjust_juxtaposing_management
int my_inst_index = 0;

// Start to manage the main window multiple instances juxtaposing.

// Il faudrait améliorer ce fonctionnement,  même si ce n'est pas fondamentale, en gérant le contenu du fichier 'instance_name', comme suit :
// Une ligne par instance, contenant : pid, x, y, w, h.
// Si le fichier est absent ou vide alors, l'instance qui le créé ou y ajoute la première ligne est alors, "l'instance initiale" qui sera la seule modifiant les préférences.
// La première ligne doit toujours correspondre à la plus ancienne instance en cours (Pas forcémment l'instance initiale) mais qui fait office de référence pour le positionnement des suivantes.
// Chaque ligne est mise à jour à chaque fois que l'instance lui correspondant est déplacer et /ou repositionnée.
// Si on quitte la plus ancienne instance alors sa ligne, étant la première du fichier, est effacée et la suivante, devenant la "nouvelle première ligne", correspondra à la nouvelle instance la plus ancienne faisant office de référence pour le
// positionnement des suivantes.
void juxtaposing_manage(const int _x, const int _y, const int _w, const int _h, bool force_ruling = false)
{
  int work_width, work_height, inst_count, inst_inc, x = _x, old_x = _x, y = _y, w = _w, h = _h;
  get_my_work_area(work_width, work_height);
  logD("JUXTA POS - work_area: (", work_width, ", ", work_height, ")");

  // La première instance du programme sert à définir le comportement d'affichage initiale des suivantes.
  // Sa géométrie permet de définir s'il y aura incrémentation ou décrémentation de la position horizontale des suivantes
  if (!std::filesystem::exists(instance_name) || std::filesystem::file_size(instance_name) == 0 || force_ruling)
  {
    inst_count = 1;
    // If twice my current windows width is still inside my screen work area then will increment x origin else will decrement it
    if (x < work_width / 2)
      inst_inc = 1;
    else
      inst_inc = -1;
  }
  else
  {
    if (std::filesystem::exists(instance_name))
    {
      // 📖 Les autres se contente donc de se juxtaposer à la première, dans les limites de la zone de travail.
      std::ifstream ifs(instance_name);
      ifs >> inst_count >> inst_inc >> x >> y >> w >> h;

      if (inst_count < 1 || inst_count > 100)
        inst_count = 1;

      if (inst_inc != 1 && inst_inc != -1)
      {
        if (x < work_width / 2)
          inst_inc = 1;
        else
          inst_inc = -1;
      }

      correct_geometry(x, y, w, h);
      old_x = x;
      ifs.close();

      // ✏️ Modification of my_inst_index and inst_count
      my_inst_index = inst_count;
      x += inst_count * inst_inc * w;
      inst_count++;

      // Cycle x origin to horizontal limits of the work area
      if (inst_inc == 1)
      {
        if (x > work_width - w)
          x = 8;
      }
      else
      {
        if (x < 0)
          x = work_width - w;
      }
    }
  }

  // Move main window
  if (!force_ruling)
  {
    correct_geometry(x, y, w, h);
    main_window->resize(x, y, w, h);
  }

  fl_message_position(main_window->x_root(), main_window->y_root() + 100, 0);

  logD("juxtaposing_manage - my_inst_index: ", my_inst_index, ", count: ", inst_count, ", inst_inc: ", inst_inc, ", x: ", old_x, ", y: ", y, ", w: ", w, ", h: ", h);

  // 💾 Écriture des données
  std::ofstream ofs(instance_name, std::ios::trunc);
  ofs << inst_count << ' ' << inst_inc << ' ' << old_x << ' ' << y << ' ' << w << ' ' << h << std::endl;
  ofs.close();
}

// Si la première instance du programme bouge, alors changement des repères de juxtaposition et redéfinition si incrémentation ou décrémentation.
int juxtaposing_update(int)
{
  if (my_inst_index == 0)
  {
    static int prev_x = -1, prev_y = -1, prev_w = -1, prev_h = -1;
    int x = main_window->x_root(), y = main_window->y_root(), w = main_window->w(), h = main_window->h();
    if (x != prev_x || y != prev_y || w != prev_w || h != prev_h)
    {
      correct_geometry(x, y, w, h);
      juxtaposing_manage(x, y, w, h, true);
      prev_x = x;
      prev_y = y;
      prev_w = w;
      prev_h = h;
    }
  }

  return 0;
}

void juxtaposing_end()
{
  static bool already_done = false;
  if (already_done)
    return;
  already_done = true;

  remove_opened();

  // Remove instance_name
  if (std::filesystem::exists(instance_name))
  {
    if (my_inst_index == 0)
    {
      logD("juxtaposing_end");
      std::filesystem::remove(instance_name);
    }
    else
    {
      // Decremente inst_count dans instance_name
      int inst_count, inst_inc, x, y, w, h;
      std::ifstream ifs(instance_name);
      ifs >> inst_count >> inst_inc >> x >> y >> w >> h;
      correct_geometry(x, y, w, h);

      if (inst_inc != 1 && inst_inc != -1)
      {
        int work_width, work_height;
        get_my_work_area(work_width, work_height);
        if (x < work_width / 2)
          inst_inc = 1;
        else
          inst_inc = -1;
      }

      ifs.close();
      inst_count--;
      if (inst_count < 1 || inst_count > 100)
        inst_count = 1;
      std::ofstream ofs(instance_name, std::ios::trunc);
      ofs << inst_count << ' ' << inst_inc << ' ' << x << ' ' << y << ' ' << w << ' ' << h << std::endl;
      ofs.close();
    }
  }
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
/*
std::string pref_get_string(Fl_Preferences &pref, const std::string key, const std::string def_val)
{
  char *pval;
  pref.get(key.c_str(), pval, def_val.c_str());
  std::string val = pval;
  free(pval);
  return val;
}
*/
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

void set_prefs()
{
  // On ne sauvegarde les préférences que de la première instance
  if (my_inst_index == 0)
  {
    logT("Saving prefs");
    window.set("xpos", main_window->x_root());
    window.set("ypos", main_window->y_root());
    window.set("width", main_window->w());
    window.set("height", main_window->h());

    window.set("theme", OS::current_theme_string().c_str());

    window.set("case", case_sensitive_find->value());

    window.set("find value", str_find->value());
    window.set("replace value", str_replace->value());
  }

  // Always merge menus
  merge_menu("find menu", str_find, str_find->value());
  merge_menu("replace menu", str_replace, str_replace->value());

  trace_prefs();

  juxtaposing_end();
}
