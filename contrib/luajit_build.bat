@rem Script to build luajit with MSVC.
@rem

@IF "%1"=="" goto :BAD

@set OUTPATH=%1
@set OUTNAME=lua51.lib

@IF exist %OUTPATH%%OUTNAME% (
	@echo Nothing to do for LuaJIT
	goto :END
)

@cd luajit/src

@echo === Building LuaJIT ===

@setlocal
call msvcbuild.bat %2 static
@if errorlevel 0 (
	@mv %OUTNAME% %LIBPATH%%OUTNAME%
	@mv vc*.pdb %LIBPATH%
	@del luajit.exe *.pdb luajit.ilk
)
@endlocal

@cd ../..

@goto :END
:BAD
@echo Missing parameter : output directory
:END
