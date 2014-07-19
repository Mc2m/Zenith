rem utilities for build

IF "%1"=="INFO"  goto :SETBUILDINF
IF "%1"=="LIB"   goto :BUILDLIB
IF "%1"=="CLEAN" goto :CLEANLIB

:SETBUILDINF
rem set the different parameters
set SOLUTIONDIR=%2
set CONFIGURATION=%3
set PLATFORM=%4

rem detect debug build
set DEBUG=0
if not "%CONFIGURATION:Debug=%"=="%CONFIGURATION%" (
	set DEBUG=1
	set CONFIGURATION=Debug
) else (
	if not "%CONFIGURATION:debug=%"=="%CONFIGURATION%" (
		set DEBUG=1
		set CONFIGURATION=Debug
	)
)

rem fetch the toolset version from input.
set TOOLSETVER=%VisualStudioVersion:.=%

rem set paths
set OUTPATH=%SOLUTIONDIR%lib\%PLATFORM%\%CONFIGURATION%\

goto :END
	
:BUILDLIB
call libs.bat %2
IF not defined PROJNAME (
	echo Missing Library %2
	exit /b 1
)

call %CONTRIBPATH%%BUILDBAT%
IF errorlevel 1 exit /b 1
goto :END

:CLEANLIB
call libs.bat %2 clean
IF not defined PROJNAME (
	echo Warning: Missing Library %2
	exit /b 0
)

call %BUILDBAT% clean

:END