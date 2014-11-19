#!/bin/bash
# store contrib build configuration

luaJIT_Prebuild() {
	echo "=== Building ${LABELNAME} ==="
	echo ""

	cd "${SCPATH}${FOLDER}"
}

luaJIT_Postbuild() {
	rm -f luajit.exe lua*.pdb *.exp *.ilk
			
	mv $LIBNAME $LIBPATH
	find ./ -type f -name '*.pdb' -exec mv '{}' $LIBPATH ';'
	if [ -f $DLLNAME ]; then mv $DLLNAME $BINPATH; fi
}

luaJIT_Clean() {
	echo -n "Cleaning ${LABELNAME}... "
	rm -f "${LIBPATH}${LIBNAME}"
	find $LIBPATH -type f -name '*.pdb' -exec rm -f '{}' ';'
	rm -f "${BINPATH}${DLLNAME}"
	echo "Done"
}

luaJIT() {
	export PROJNAME="lua51"
	export FOLDER="luajit/src/"
	export LABELNAME="LuaJIT"
	
	if [ ! $CLEAN ]; then
		export CONFIG=""
		if [ $DEBUG ]; then CONFIG="debug"; fi
		if [ $STATIC ]; then CONFIG="${CONFIG} static"; fi
		
		export PREBUILDEVENT="luaJIT_Prebuild"
		export BUILDEVENT="cmd.exe \/c \"msvcbuild.bat ${CONFIG}\""
		export POSTBUILDEVENT="luaJIT_Postbuild"
		CONFIG=""
	else
		export CLEANEVENT="luaJIT_Clean"
	fi
	
	autobuildBuild
}

lux() {
	export PROJNAME="lux"
	export FOLDER="lux/"
	export LABELNAME="Lux"
	
	if [ ! $CLEAN ]; then
		export PREPROCDEF="/D _CRT_SECURE_NO_WARNINGS"
		export COMPILEOPTS="/I\"../luajit/src\" /W3 /MP /Oy- /Gm- /TC /GF"
		export SRCS="*.c"
		
		if [ $DEBUG ]; then
			export COMPILEOPTS="$COMPILEOPTS /ZI /Od /RTC1 /sdl /WX"
		else
			export COMPILEOPTS="$COMPILEOPTS /GL /Gy /Ox /GS /WX-"
		fi
	fi
	
	autobuildCustomBuild
}

luaJIT
lux
