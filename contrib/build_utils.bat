rem utilities for build

IF "%1"=="INFO" goto :SETBUILDINF
IF "%1"=="LIB"  goto :BUILDLIB

:SETBUILDINF
rem set the different parameters
set SOLUTIONDIR=%2
set CONFIGURATION=%3
set PLATFORM=%4

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

rem set paths
set OUTPATH=%SOLUTIONDIR%lib\%PLATFORM%\%CONFIGURATION%\

goto :END
	
:BUILDLIB
IF not exist libs.bat (
	echo Missing libs.bat
	exit /b 1
)

call libs.bat %2

IF not defined FOLDER (
	echo Missing Library %2
	exit /b 1
)

:BUILDLIBLOOP
rem make sure the directory isn't empty
for /F %%i in ('dir /a-d "%FOLDER%\*"') do (
	rem not empty
	call %CONTRIBPATH%%BUILDBAT%
	IF errorlevel 1 exit /b 1
	goto :END
)

rem empty
set OLDDIR = "%CD%"
cd ../..
IF %OLDDIR%=="%CD%" (
	set "OLDDIR="
	echo Can't find the folder %FOLDER% for library %2
	exit /b 1
)
set "OLDDIR="
goto :BUILDLIBLOOP

:END
