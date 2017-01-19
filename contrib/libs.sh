#!/bin/bash
# store contrib build configuration

luaJIT_Prebuild() {
	echo "=== Building ${ABLABELNAME} ==="
	echo ""

	cd "${ABSCPATH}${FABOLDER}"
}

luaJIT_Postbuild() {
	rm -f luajit.exe lua*.pdb *.exp *.ilk
			
	mv $ABLIBNAME $ABLIBPATH
	find ./ -type f -name '*.pdb' -exec mv '{}' $ABLIBPATH ';'
	if [ -f $ABDLLNAME ]; then mv $ABDLLNAME $ABBINPATH; fi
}

luaJIT_Clean() {
	echo -n "Cleaning ${ABLABELNAME}... "
	rm -f "${ABLIBPATH}${ABLIBNAME}"
	find $ABLIBPATH -type f -name '*.pdb' -exec rm -f '{}' ';'
	rm -f "${BINPATH}${DLLNAME}"
	echo "Done"
}

luaJIT() {
	export PABROJNAME="lua51"
	export ABFOLDER="luajit/src/"
	export ABLABELNAME="LuaJIT"
	
	if [ ! $ABCLEAN ]; then
		export CONFIG=""
		if [ $ABDEBUG ]; then CONFIG="debug"; fi
		if [ $ABSTATIC ]; then CONFIG="${CONFIG} static"; fi
		
		export ABPREBUILDEVENT="luaJIT_Prebuild"
		export ABBUILDEVENT="cmd.exe \/c \"msvcbuild.bat ${CONFIG}\""
		export ABPOSTBUILDEVENT="luaJIT_Postbuild"
		CONFIG=""
	else
		export ABCLEANEVENT="luaJIT_Clean"
	fi
	
	autobuildBuild
}

lux() {
	export ABPROJNAME="lux"
	export ABFOLDER="lux/"
	export ABLABELNAME="Lux"
	
	if [ ! $ABCLEAN ]; then
		export ABPREPROCDEF="/D _CRT_SECURE_NO_WARNINGS"
		export ABCOMPILEOPTS="/I\"../luajit/src\" /W3 /MP /Oy- /Gm- /TC /GF"
		export ABSRCS="*.c"
		
		if [ $ABDEBUG ]; then
			export COMPILEOPTS="$COMPILEOPTS /ZI /Od /RTC1 /sdl /WX"
		else
			export COMPILEOPTS="$COMPILEOPTS /GL /Gy /Ox /GS /WX-"
		fi
	fi
	
	autobuildCustomBuild
}

luaJIT
lux

