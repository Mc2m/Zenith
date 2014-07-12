@rem Script to build contribs with MSVC.
@rem

@if not defined INCLUDE goto :FAIL

@echo Building Zenith contribs

@rem %1 is output path, %2 is debug or release, %3 is architecture
@IF "%1"=="" goto :BAD
@IF "%2"=="" goto :BAD
@IF "%3"=="" goto :BAD

@rem fetch the contrib directory
@set CBPATH=%~dp0

@rem fetch the directory where library should be put
@set LIBPATH=%1

@rem create the folder to be on the safe side
@IF not exist %LIBPATH% @mkdir "%LIBPATH%"

@cd %CBPATH%

@rem build lua jit
@call "%CBPATH%luajit_build.bat" %LIBPATH% %2 %3


@IF errorlevel 0 @echo Build is successful
goto :END

:BAD
@echo Missing library path
@set ERRORLEVEL=1
goto :END
:FAIL
@echo You must open a "Visual Studio .NET Command Prompt" to run this script
@set ERRORLEVEL=1
:END
