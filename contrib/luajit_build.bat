rem Script to build luajit with MSVC.
rem

setlocal

set PROJNAME=lua51
set OUTPUT=%OUTPATH%%PROJNAME%.lib

IF "%1"=="clean" goto :CLEAN

IF exist %OUTPUT% (
	echo Nothing to do for LuaJIT
	goto :END
)

echo === Building LuaJIT ===
echo.

IF %DEBUG%==1 set CONFIG=debug

cd luajit/src
call msvcbuild.bat %CONFIG% static
if errorlevel 0 (
	mv %PROJNAME%.lib %OUTPUT%
	IF %DEBUG%==1 mv vc%TOOLSETVER%.pdb %OUTPATH%
	del /F /Q luajit.exe *.pdb *.exp luajit.ilk
)
cd ../..

endlocal

goto :END

:CLEAN
del /F /Q %OUTPUT% %OUTPATH%*.pdb
endlocal
goto :END

:END
