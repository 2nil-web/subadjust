
#include <algorithm>
#include <cctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif

#include "app_info.h"
#include "options.h"
#include "utils.h"

option_info::option_info(char p_short_name, std::string p_long_name, t_opt_func p_func, std::string p_help, e_option_mode p_mode, e_option_env p_env)
{
  short_name = p_short_name;
  long_name = p_long_name;
  func = p_func;
  help = p_help;
  mode = p_mode;
  env = p_env;
}

option_info::option_info(const std::string p_help)
{
  mode = only_help;
  help = p_help;
}

std::ostream &operator<<(std::ostream &os, option_info &opt)
{
#ifdef FULL_OUTPUT
  os << "short_name:" << opt.short_name << ", long_name:[" << opt.long_name << "], help:[" << opt.help << "], mode:" << opt.mode << ", env:" << opt.env << std::endl;

#else

  os << "mode:" << opt.mode << ", env:" << opt.env << ", ";

  if (opt.func)
    os << "has A  func, ";
  else
    os << "has NO func, ";

  if (opt.mode != only_help)
  {
    if (opt.short_name != 0)
      os << "short_name:[" << opt.short_name << "], ";
    if (!opt.long_name.empty())
      os << "long_name:[" << opt.long_name << "], ";
  }

  if (!opt.help.empty())
    os << "help:[" << opt.help << ']';
  os << std::endl;
#endif
  return os;
}

void options::set(int argc, char **argv, opti_dq p_opt_inf)
{
  Progname = progname = std::filesystem::path(argv[0]).stem().string();
  Progname[0] = std::toupper(Progname[0]);

  args.insert(args.begin(), argv + 1, argv + argc);
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(int argc, char **argv, opti_dq p_opt_inf)
{
  set(argc, argv, p_opt_inf);
}

void options::set(std::string p_progname, arg_dq l_args, opti_dq p_opt_inf)
{
  if (!progname.empty())
    progname = p_progname;
  if (!args.empty())
    args = l_args;
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(std::string p_progname, arg_dq l_args, opti_dq p_opt_inf)
{
  set(p_progname, l_args, p_opt_inf);
}

std::string options::version()
{
  std::string vers = Progname + " version " + app_info.version;
  if (!app_info.decoration.empty())
    vers += ' ' + app_info.decoration;

  vers += '\n' + app_info.copyright;

  if (traceability_enabled)
  {
    std::string trac;
    if (!app_info.commit.empty())
      trac += "\nCommit " + app_info.commit + ". ";

    trac += "Built for " + app_info.platform;
    if (!app_info.created_at.empty())
      trac += " on " + app_info.created_at;

    vers += trac;
  }

  return vers;
}

std::ostream &options::version(std::ostream &os)
{
  os << version() << std::endl;

  return os;
}

// Insert a substring in a string at every space before 'l' position ...
std::string in_frame(std::string s, std::string ss, size_t l = 80)
{
  std::string res = {};
  size_t last_space_pos = 0;
  for (size_t i = 0; i < s.size(); i++)
  {
    if (i > 0 && (i % l) == 0)
    {
      // Insert substring at last space position
      res.insert(last_space_pos, ss);
    }

    res += s[i];
    if (res[i] == ' ')
      last_space_pos = i;
  }

  return res;
}

void options::set_desc(std::string _desc)
{
  desc = _desc;
}

extern bool gui_mode;
std::string options::usage_opt(size_t max_width)
{
  std::string usage;

  if (gui_mode)
    usage = "";
  else
    usage = version() + '\n';

  if (!desc.empty())
    usage += desc + "\n\n";

  usage += "Usage: " + progname + " [OPTIONS] ARGUMENT\n";
  usage += "Available options\n";

  size_t longest_opt = 0;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        size_t curr_l = opt.long_name.size() + 9;
        if (opt.mode == optional)
          curr_l += 6;
        if (opt.mode == required)
          curr_l += 4;
        if (curr_l > longest_opt)
          longest_opt = curr_l;
      }
    }
  }

  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      std::string opt_s = {};
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        if (opt.short_name != 0)
          opt_s += " -" + std::string(1, opt.short_name);
        else
          opt_s += "  ";

        if (!opt.long_name.empty())
        {
          if (opt.short_name != 0)
            opt_s += ',';
          else
            opt_s += ' ';
          opt_s += " --" + opt.long_name;
        }

        if (opt.mode == optional)
          opt_s += " [ARG]";
        if (opt.mode == required)
          opt_s += " ARG";
        usage += opt_s + std::string(longest_opt - opt_s.size(), ' ');
      }

      if (max_width > 0)
        usage += in_frame(opt.help, '\n' + std::string(longest_opt + 1, ' '), max_width - longest_opt);
      else
        usage += opt.help;
      usage += '\n';
    }
  }

  return usage;
}

std::string options::usage_int(size_t max_width)
{
  std::string usage = {};

  usage += "Available commands/and their shortcut\n";

  size_t longest_opt = 0;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        size_t curr_l = opt.long_name.size() + 3;
        if (opt.mode == optional)
          curr_l += 6;
        if (opt.mode == required)
          curr_l += 4;
        if (curr_l > longest_opt)
          longest_opt = curr_l;
      }
    }
  }

  bool first = true;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (!first)
        usage += '\n';
      else
        first = false;

      std::string opt_s = {};
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        if (!opt.long_name.empty())
        {
          opt_s += opt.long_name;
        }

        if (opt.short_name != 0)
        {
          opt_s += "/" + std::string(1, opt.short_name);
        }

        if (opt.mode == optional)
          opt_s += " [ARG]";
        if (opt.mode == required)
          opt_s += " ARG";
        usage += opt_s + std::string(longest_opt - opt_s.size(), ' ');
      }

      if (max_width > 0)
        usage += in_frame(opt.help, '\n' + std::string(longest_opt + 1, ' '), max_width - longest_opt);
      else
        usage += opt.help;
    }
  }

  return usage;
}

std::string options::usage(size_t max_width)
{
  if (imode)
    return usage_int(max_width);
  else
    return usage_opt(max_width);
}

std::ostream &options::usage(std::ostream &os, size_t max_width)
{
  os << usage(max_width) << std::endl;
  return os;
}

// Add -h/-help/--help and -v/-version/--version
void options::add_default()
{
  bool no_v = true, no_h = true, no_t = true;

  // Check if the options -H/--help, -V/--version or --traceability are overwritten, if so they are not defined here.
  for (auto opt : opt_inf)
  {
    if (opt.short_name == 'H' || opt.long_name == "help")
      no_h = false;
    if (opt.short_name == 'V' || opt.long_name == "version")
      no_v = false;
    if (opt.long_name == "traceability")
      no_t = false;
  }

  if (no_h)
  {
    opt_inf.push_front(option_info(
        'H', "help",
        [this](s_opt_params &) -> void {
          // Could use ncurse (pdcurses.org) to define max_width as the console window width ... Defaulting to 100
          gui_mode = false;
          usage(std::cout, 0);
          if (!imode)
            exit_after_opt = true;
        },
        "Display this message and exit. This option implies -V/--version."));
  }
  if (no_v)
  {
    opt_inf.push_front(option_info(
        'V', "version",
        [this](s_opt_params &) -> void {
          gui_mode = false;
          version(std::cout);
          if (!imode)
            exit_after_opt = true;
        },
        "Output version information and exit."));
  }

  if (no_t)
  {
    opt_inf.push_front(option_info(
        0, "traceability",
        [this](s_opt_params &) -> void {
          traceability_enabled = true;
          gui_mode = false;
          version(std::cout);
          if (!imode)
            exit_after_opt = true;
        },
        "SECRET_OPTION provided for traceability when needed (debug)."));
  }
}

void options::run_opt(option_info opt)
{
  s_opt_params p({opt.short_name, opt.long_name, {}, 0});

  if (opt.mode == optional || opt.mode == required)
  {
    if (it_args != p_args.end() - 1)
    {
      p.val = *(it_args + 1);
      opt.func(p);
      // if (p.ret)
      it_args++;
    }
    else if (opt.mode == e_option_mode::required)
    {
      std::cerr << "Missing argument to '" << opt.short_name << '/' << opt.long_name << "', ignoring this option." << std::endl;
    }
  }
  else
  {
    opt.func(p);
  }
}

void options::run_opt(char short_name)
{
  for (auto opt : opt_inf)
  {
    if (opt.short_name == short_name)
    {
      run_opt(opt);
      return;
    }
  }

  std::cerr << "Uknown short option '-" << short_name << "', ignoring it." << std::endl;
}

void options::run_opt(std::string long_name)
{
  for (auto opt : opt_inf)
    if (opt.long_name == long_name)
    {
      run_opt(opt);
      return;
    }

  std::cerr << "Uknown long option '--" << long_name << "', ignoring it." << std::endl;
}

void options::parse()
{
  p_args = args;
  args.clear();

  bool vers_exists = false, help_exists = false, trac_exists = false, rem_vers = false;
  arg_iter it_vers, it_trac;
  // Anticipate the -H/--help or --traceability options to eventually remove the -V/--version option that is implied with them
  for (it_args = p_args.begin(); it_args != p_args.end(); it_args++)
  {
    if ((*it_args) == "-V" || (*it_args) == "-version" || (*it_args) == "--version")
    {
      vers_exists = true;
      it_vers = it_args;
    }

    if ((*it_args) == "-H" || (*it_args) == "-help" || (*it_args) == "--help")
    {
      help_exists = true;
      rem_vers = true;
    }

    if ((*it_args) == "--traceability")
    {
      traceability_enabled = true;
      trac_exists = true;
      it_trac = it_args;
      rem_vers = true;
    }
  }

  // Then if necessary, search and remove -V/--version
  if (rem_vers)
  {
    if (vers_exists)
      p_args.erase(it_vers);
    if (help_exists && trac_exists)
      p_args.erase(it_trac);
  }

  // Now process all the options
  for (it_args = p_args.begin(); it_args != p_args.end(); it_args++)
  {
    if ((*it_args)[0] == '-')
    {
      // Simple dash alone
      if (it_args->size() == 1)
      {
        args.push_back(*it_args); // Not option, remaining arg
      }
      // Simple dash not alone
      else if (it_args->size() == 2)
      {
        // Double dash alone
        if ((*it_args)[1] == '-')
        {
          args.push_back(*it_args); // Not option, remaining arg
        }
        // Simple dash with a short option
        else
        {
          run_opt((*it_args)[1]);
        }
      }
      else if (it_args->size() > 2)
      {
        // double dash with a long option
        if ((*it_args)[1] == '-')
        {
          run_opt((*it_args).substr(2));
        }
        // Simple dash with a long option OR multiple short options
        else
        {
          // run_opt((*it_args).substr(1));
          for (size_t i = 1; (*it_args)[i] && !std::isspace((*it_args)[i]); i++)
          {
            run_opt((*it_args)[i]);
          }
        }
      }
    }
    else
    {
      args.push_back(*it_args); // Not option, remaining arg
    }
  }

  if (exit_after_opt)
    exit(0);
}

void options::parse(std::istream &is)
{
  imode = true;

  std::string s, r1, r2;
  s_opt_params op;
  bool unknown_cmd;

  for (;;)
  {
    if (&is == &std::cin)
      std::cout << prompt;

    if (!std::getline(is, s))
      break;

    trim(s);

    if (!s.empty() && s[0] > ' ')
    {
      split_1st(r1, r2, s);

      unknown_cmd = true;
      for (auto opt : opt_inf)
      {
        if (r1[0] == opt.short_name || r1 == opt.long_name)
        {
          op = {opt.short_name, opt.long_name, r2, 0};
          if (r2.empty() && opt.mode == e_option_mode::required)
          {
            std::cerr << "Missing argument to '" << opt.short_name << '/' << opt.long_name << "', ignoring this command." << std::endl;
          }
          else
          {
            opt.func(op);
          }

          unknown_cmd = false;
          break;
        }
      }

      if (unknown_cmd)
      {
        std::cerr << "Unknown command '" << s << "', ignoring it." << std::endl;
      }
    }
  }

  imode = false;
}

void options::parse(std::filesystem::path path)
{
  std::ifstream file(path, std::ios::binary);
  if (file)
  {
    //    std::cout << "Parsing file " << path << std::endl;
    parse(file);
  }
  file.close();
}

std::ostream &operator<<(std::ostream &os, options &opts)
{
  opts.version(os);

  for (auto opt : opts.opt_inf)
  {
    os << opt;
  }

  return os;
}
