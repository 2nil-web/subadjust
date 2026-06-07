# SubAdjust

##### ***A graphical and command line tool to modify the timeline of subtitles files in SubRip (.srt) and WEBVTT (.vtt) format***

<sup>*Actually available in 3 languages : english, french and spanish. But others can easily be added.*</sup>

Can be used as a file converter between SubRip and WEBVTT format.

Also allows to export to CSV format.

##### ***You can download a binary version of this tool in the "[Release](https://github.com/2nil-web/subadjust/releases)" link.***

<sup>*Available to five Linux ![Linux](assets/Tux.svg "Linux") distros (Arch ![Arch](assets/Arch.svg "Arch"), Debian ![Debian](assets/Debian.svg "Fedora"), Fedora ![Fedora](assets/Fedora.svg "Fedora"), OpenSUSE ![OpenSUSE](assets/OpenSUSE.svg "OpenSUSE"), Ubuntu ![Ubuntu](assets/Ubuntu.svg "Ubuntu")) and Windows ![Windows](assets/Windows.svg "Windows").*</sup>

<sup>*It should also be possible to build and deliver it for macOS ![macOS](assets/MacOS.svg "MacOS").*</sup>

After the download, just run the setup or unzip the zip ball and run the app with .srt or .vtt files.

### ***The Graphical User Interface aims to be self-explanatory***

***But some indications might be useful***

![](assets/QuickDoc.svg "Quick doc")

*<sup>Many tooltips will guide you during the use of this GUI.</sup>*

<ins>***There is also an inline help :***</ins>

```
Subadjust version 1.1.1
Copyright © D. LALANNE - MIT License - No warranty of any kind.
A tool that allows to process subtitles text files.
The batch mode allows processing at the command line or by script.
Meanwhile the GUI mode adds a search and replace feature with regular expressions.

Usage: subadjust [OPTIONS] ARGUMENT
Available options
 -v, --version             Output version information and exit.
 -h, --help                Display this message and exit. This option implies -V/--version.

 -g, --gui-mode            Process the input file and show it with the gui, this is the default behavior.
 -c, --batch-mode          Process the input the file and print the result.
 -f, --input file ARG      Name of the file to read. It is the same than directly passing a file name as an argument without this option.
 -o, --output-file ARG     Process the input file in batch mode and write the result into the file whose name is passed in argument.
    Based on the file extension, may write it to SubRip (.srt), WEBVTT (.vtt) or even CSV (.csv) format.
 -i, --modify-input        Write the processing result into the same input file. Only has a meaning in batch mode, ignored in GUI mode.

 -r, --reset-pref          Reset the configuration to default values.

 -b, --begin-time ARG      Change the beginning time stamp to the provided argument.
 -e, --end-time ARG        Change the end time stamp to the provided argument.
 -k, --duration-coeff ARG  Change the duration coefficient to the provided argument.
 -a, --start-offset ARG    Change the start offset to the provided argument.
 -s, --stop-offset ARG     Change the stop offset to the provided argument.
These 5 previous options are processed after reading the file and have effect in both GUI and batch mode.

 -l, --log-level ARG       Set the level of the log messages to display :
    ALL   All the messages.
    TRACE Almost all messages, at least those finer than the INFO level.
    INFO  Informational messages that highlight the application's progress at a coarser level.
    DEBUG Fine-grained events, the most useful for debugging an application.
    WARN  Potentially dangerous situations.
    ERROR Errors that might still allow the application to continue running.
    FATAL Very serious errors that will likely cause the application to crash.
    OFF   Disables logging.
 -m, --log-file ARG        Define the file where log messages will be stored.
    Default it to store them in ...subadjust.log
    The special value 'console' will allows to output the log messages to the console, if possible.
```

### ***Thanks and copyrights:***

- To [Bill Spitzak](https://github.com/spitzak) and others, of course, for the wonderful [FLTK](https://www.fltk.org) tool.
- To [**Rangi42**](https://github.com/Rangi42), [**dannye**](https://github.com/dannye) and [Remy Oukaour](https://github.com/roukaour) for the fltk graphical theme enhancement, which I've somewhat hacked.

### ***Technical insight:***

It compiles and run under both Windows (With Visual Studio and g++) and Linux (With g++), thanks to FLTK.

There is a "help" option in the Makefile that roughly explain the build and delivery workflow.

Although untested, this process should work identically under macOS, provided the necessary prerequisites are met.

###### Prerequisites to the build:

- A posix environment:
   [Linux](https://www.kernel.org/) of course.
   And for [Windows]([http://windows.microsoft.com), preferably [msys2](https://www.msys2.org) but [cygwin](https://www.cygwin.com/) should also work.
- [FLTK](https://www.fltk.org/)
- [g++](https://gcc.gnu.org)  (For Windows and linux)
- [ImageMagick](https://imagemagick.org)
- [Inkscape](https://inkscape.org)
- [libjpeg](https://www.ijg.org)
- [librsvg2](https://gitlab.gnome.org/GNOME/librsvg)
- [make](https://www.gnu.org/s/make)
- [pandoc](https://pandoc.org/)
- [gettext](https://www.gnu.org/s/gettext)
- [libiconv](https://www.gnu.org/software/libiconv)
- [Visual C++](https://docs.microsoft.com/fr-fr/cpp/) should be enough if you only want to compile under Windows, as .slnx/.vcxproj files are provided.
