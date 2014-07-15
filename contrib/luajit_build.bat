rem Script to build luajit
rem

setlocal
set OUTPUT=%OUTPATH%%PROJNAME%.lib

IF "%1"=="clean" goto :CLEAN

IF exist %OUTPUT% (
	echo Nothing to do for %LABELNAME%
	goto :END
)

echo === Building %LABELNAME% ===
echo.

IF %DEBUG%==1 set CONFIG=debug

cd %FOLDER%/src
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
2>NUL del /F /Q %OUTPUT% %OUTPATH%*.pdb
endlocal
goto :END

:END
