
rem clean
set "FOLDER="
set "BUILDBAT="
set "PROJNAME="
set "COMPILEOPTS="
set "PREPROCDEF="
set "PCH="
set "PCHSRC="
set "SRCS="
set "LIBOPTS="
set "PREBUILDEVENT="


IF "%1"=="luajit" goto :LUAJIT
IF "%1"=="lux"    goto :LUX

goto :END

:LUAJIT

set PROJNAME=lua51
set BUILDBAT=luajit_build.bat

IF "%2"=="clean" goto :END

set FOLDER=luajit
set LABELNAME=LuaJIT

goto :END

:LUX

set PROJNAME=lux

IF "%2"=="clean" goto :DEFAULT

set FOLDER=lux
set LABELNAME=Lux
set PREPROCDEF=/D _CRT_SECURE_NO_WARNINGS
set COMPILEOPTS= /I"luajit/src" /W3 /MP /Oy- /Gm- /TC /GF
set "SRCS=lux\*.c"

IF %DEBUG%==1 (
	set COMPILEOPTS=%COMPILEOPTS% /ZI /Od /RTC1 /sdl /WX
) else (
	set COMPILEOPTS=%COMPILEOPTS% /GL /Gy /Ox /GS /WX-
)

goto :DEFAULT

:DEFAULT

set BUILDBAT=contrib_build.bat

IF "%2"=="clean" goto :END

IF %DEBUG%==1 (
	set COMPILEOPTS=%COMPILEOPTS% /MTd /fp:precise
	set PREPROCDEF=%PREPROCDEF% /D _DEBUG /D DEBUG
) else (
	set COMPILEOPTS=%COMPILEOPTS% /MT /fp:precise
	set PREPROCDEF=%PREPROCDEF% /D NDEBUG
	set LIBOPTS=%LIBOPTS% /LTCG
)

:END
