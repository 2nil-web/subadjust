set MY_HOME= %USERPROFILE%\Documents\home
PATH %PATH%;C:\Users\dplal\Documents\home\10-runset\build\msvc\win\x64\Release
PATH %PATH%;C:\Program Files\Vim\vim92
set MSYSDIR=C:\UnixTools\msys64
PATH %PATH%;%MSYSDIR%\usr\bin
set EDITOR=gvim

cscript %MY_HOME%\00-subadjust\assets\multitabexplorer.js

runset "%EDITOR% %APPDATA%\dplalanne.fr\subadjust.prefs" 3160 160 690 530
::runset "%EDITOR% %USERPROFILE%\.subadjust\juxtaposing_management" 3245 732 600 160
runset "%EDITOR% %USERPROFILE%\.subadjust\already_opened" 3160 682 690 360

cd /D %USERPROFILE%\.subadjust
mintty -p 1920,205 -s 174x25 -i %MSYSDIR%\ucrt64.ico -
::e tail -f %LOCALAPPDATA%\Temp\subadjust.log
mintty -p 1920,625 -s 174x25 -i %MSYSDIR%\ucrt64.ico -

