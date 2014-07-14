
IF "%1"=="luajit" goto :LUAJIT

goto :NONE

:LUAJIT
set FOLDER=luajit
set BUILDBAT=%FOLDER%_build.bat
goto :END

:NONE

set "FOLDER="
set "BUILDBAT="

:END
