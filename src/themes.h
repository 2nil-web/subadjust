#ifndef THEMES_H
#define THEMES_H

// Hacked from : https://github.com/Rangi42/tilemap-studio and https://github.com/roukaour/viz-brain-visualizer

#include <functional>
#include <vector>

#ifdef _MSVC_LANG
#pragma warning(push, 0)
#endif
#include <FL/Enumerations.H>
#ifdef _MSVC_LANG
#pragma warning(pop)
#endif

#include "utils.h"

class OS
{
public:
  static std::string themes_string();
  static std::string current_theme_string();
  static bool use_theme(std::string stheme = {});
  static bool use_theme(int theme);

private:
  static int _current_theme;
  static bool _is_consolas;
  static void use_classic_theme();
  static void use_aero_theme();
  static void use_metro_theme();
  static void use_aqua_theme();
  static void use_greybird_theme();
  static void use_ocean_theme();
  static void use_blue_theme();
  static void use_olive_theme();
  static void use_rose_gold_theme();
  static void use_dark_theme();
  static void use_brushed_metal_theme();
  static void use_high_contrast_theme();

  static int default_theme();
  static int theme_from_string(std::string str);

#ifdef _WIN32
  static bool is_classic_windows();
  static bool is_modern_windows();
#endif
  static constexpr bool is_dark_theme(int t);
  static void use_native_fonts();
  static void use_native_settings();

  static std::vector<std::string> themeStrVec;
  static std::vector<std::function<void()>> theme_funcs;
};

#endif
