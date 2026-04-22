PATH %PATH%;C:\Users\dplal\Documents\home\01-runset\build\msvc\win\x64\Release
PATH %PATH%;C:\Program Files\Vim\vim92
set MSYSDIR=C:\UnixTools\msys64
PATH %PATH%;%MSYSDIR%\usr\bin
set EDITOR=gvim


cd /D %USERPROFILE%\Documents\home\00-subadjust\src
start .
cd /D %USERPROFILE%\.subadjust\
start .

runset "%EDITOR% %APPDATA%\dplalanne.fr\subadjust.prefs" 3160 160 690 530
::runset "%EDITOR% %USERPROFILE%\.subadjust\juxtaposing_management" 3245 732 600 160
runset "%EDITOR% %USERPROFILE%\.subadjust\.already_opened" 3160 682 690 360

cd /D %USERPROFILE%\.subadjust
mintty -p 1920,205 -s 174x25 -i %MSYSDIR%\ucrt64.ico -
::e tail -f %LOCALAPPDATA%\Temp\subadjust.log
mintty -p 1920,625 -s 174x25 -i %MSYSDIR%\ucrt64.ico -

