PATH %PATH%;C:\Users\dplal\Documents\home\01-runset\build\msvc\win\x64\Release
PATH %PATH%;C:\Program Files\Vim\vim92
set MSYSDIR=C:\UnixTools\msys64
PATH %PATH%;%MSYSDIR%\usr\bin
set EDITOR=gvim

runset "%EDITOR% %APPDATA%\dplalanne.fr\subadjust.prefs" 3245 0 600 440

runset "%EDITOR% %MSYSDIR%\tmp\subadjust_juxtaposing_management" 3245 432 600 160
runset "%EDITOR% %MSYSDIR%\tmp\already_opened" 3245 582 600 160

runset "%EDITOR% %LOCALAPPDATA%\Temp\subadjust_juxtaposing_management" 3245 732 600 160
runset "%EDITOR% %LOCALAPPDATA%\Temp\already_opened" 3245 882 600 160

mintty -p 1920,200 -s 185x25 -i %MSYSDIR%\ucrt64.ico -
::e tail -f %LOCALAPPDATA%\Temp\subadjust.log
mintty -p 1920,620 -s 185x25 -i %MSYSDIR%\ucrt64.ico -
cd /D %USERPROFILE%\Documents\home\00-fltk_subadjust
explorer .
