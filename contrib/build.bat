@echo off

rem Script to build contribs with MSVC.
rem
setlocal

set CONTRIBLIST=(luajit)

IF "%1"=="clean" goto :CLEAN

if not defined INCLUDE goto :WRONG

rem %1 is output path, %2 is debug or release, %3 is architecture
IF "%1"=="" goto :BAD
IF "%2"=="" goto :BAD
IF "%3"=="" goto :BAD

rem set the different parameters
set SOLUTIONDIR=%1
set CONFIGURATION=%2
set PLATFORM=%3

rem detect debug build
set DEBUG=0
if not x%CONFIGURATION:Debug=%==x%CONFIGURATION% (
	set DEBUG=1
	set CONFIGURATION=Debug
) else (
	if not x%CONFIGURATION:debug=%==x%CONFIGURATION% (
		set DEBUG=1
		set CONFIGURATION=Debug
	)
)

rem fetch the toolset version from input.
set TOOLSETVER=%VisualStudioVersion:.=%

rem set output path
set OUTPATH=%SOLUTIONDIR%lib\%PLATFORM%\%CONFIGURATION%\

rem create the folder to be on the safe side
IF not exist %OUTPATH% mkdir "%OUTPATH%"

rem fetch the contrib directory
set CBPATH=%~dp0
cd %CBPATH%s

rem build contribs
for %%i in %CONTRIBLIST% do (
	call %%i_build.bat
	IF errorlevel 1 goto :FAIL
)

endlocal
goto :END

:FAIL
echo.
echo *******************************************************
echo *** Build FAILED -- Please check the error messages ***
echo *******************************************************

endlocal
exit /b 1

:CLEAN

IF "%2"=="" goto :BAD
IF "%3"=="" goto :BAD
IF "%4"=="" goto :BAD

rem set the different parameters
set SOLUTIONDIR=%2
set CONFIGURATION=%3
set PLATFORM=%4

rem set output path
set OUTPATH=%SOLUTIONDIR%lib\%PLATFORM%\%CONFIGURATION%\

rem fetch the contrib directory
set CBPATH=%~dp0
cd %CBPATH%

echo Cleaning...
for %%i in %CONTRIBLIST% do (
	call %%i_build.bat clean
)
echo Done.

endlocal
exit /b 0

:BAD
echo Missing library path
endlocal
exit /b 1

:WRONG
echo You must open a "Visual Studio .NET Command Prompt" to run this script
endlocal
exit /b 1

:END
