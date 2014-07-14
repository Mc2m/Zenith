rem Script to build contribs with MSVC.
rem

@echo off

setlocal

set CONTRIBLIST=(luajit)
set CURDIR="%CD%"
set CONTRIBPATH=%~dp0

IF "%1"=="clean" goto :CLEAN

if not defined INCLUDE goto :WRONG

rem %1 is output path, %2 is debug or release, %3 is architecture
IF "%1"=="" goto :BAD
IF "%2"=="" goto :BAD
IF "%3"=="" goto :BAD

rem set the different parameters
cd %CONTRIBPATH%
call build_utils.bat INFO %1 %2 %3

rem create the folder to be on the safe side
IF not exist %OUTPATH% mkdir "%OUTPATH%"

rem build contribs
for %%i in %CONTRIBLIST% do (
	cd %CONTRIBPATH%
	call build_utils.bat LIB %%i
	IF errorlevel 1 goto :FAIL
)

cd %CURDIR%
endlocal
goto :END

:CLEAN

IF not defined %1 goto :BAD
IF not defined %2 goto :BAD
IF not defined %3 goto :BAD

rem set the different parameters
call build_utils.bat INFO %2 %3 %4

cd %CONTRIBPATH%

echo Cleaning...
for %%i in %CONTRIBLIST% do (
	call %%i_build.bat clean
)
echo Done.

cd %CURDIR%

endlocal
exit /b 0

:FAIL

cd %CURDIR%
echo.
echo *******************************************************
echo *** Build FAILED -- Please check the error messages ***
echo *******************************************************

endlocal
exit /b 1

:BAD
echo Missing argument
endlocal
exit /b 1

:WRONG
echo You must open a "Visual Studio .NET Command Prompt" to run this script
endlocal
exit /b 1

:END
