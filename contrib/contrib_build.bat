rem generic script to build a contrib with MSVC.
rem

setlocal
set OUTPUT=%OUTPATH%%PROJNAME%.lib

IF "%1"=="clean" goto :CLEAN

IF exist %OUTPUT% (
	endlocal
	echo Nothing to do for %LABELNAME%
	goto :END
)

echo === Building %LABELNAME% ===
echo.

rem set intermediate directory
set INTPATH=%SOLUTIONDIR%obj\%CONFIGURATION%\%PROJNAME%\
IF not exist %INTPATH% mkdir "%INTPATH%"

%PREBUILDEVENT%

set COMPILE=cl /nologo /c /Fo%INTPATH% /Fd%INTPATH%vc%TOOLSETVER%.pdb
set LINK=lib /nologo /nodefaultlib

if DEFINED PCH (
%COMPILE% %COMPILEOPTS% %PREPROCDEF% %PCHSRC% /Yc"%PCH%" /Fp%INTPATH%%PROJNAME%.pch
IF errorlevel 1 goto :FAIL
set COMPILEOPTS=%COMPILEOPTS% /Yu"%PCH%" /Fp%INTPATH%%PROJNAME%.pch
)

%COMPILE% %COMPILEOPTS% %PREPROCDEF% %SRCS%
IF errorlevel 1 goto :FAIL
%LINK% /OUT:%OUTPUT% %INTPATH%*.obj %LIBOPTS%
IF errorlevel 1 goto :FAIL

echo.
echo === Successfully built %LABELNAME% for Windows/%CONFIGURATION% ===
endlocal

goto :END

:CLEAN
set INTPATH=%SOLUTIONDIR%obj\%CONFIGURATION%\%PROJNAME%\
2>NUL rm -rf %INTPATH% %OUTPUT%
endlocal
goto :END

:FAIL
echo.
echo ! Build Failed !
endlocal
exit /b 1

:END
