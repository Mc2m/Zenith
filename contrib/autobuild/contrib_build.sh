#!/bin/bash
#
#	contrib-build 
#
#	build a contrib
#

#
#	setupBuild 
#	setup informations needed for the build
#

export ABCOMPILER=""
export ABLINKER=""
export ABLINKERDEFOPTS=""

#
#	autobuildReset 
#	reset Build options
#
autobuildReset() {
	ABCUSTOMBUILD=""
	ABPROJNAME=""
	ABFOLDER=""
	ABLABELNAME=""
	ABCOMPILEOPTS=""
	ABPREPROCDEF=""
	ABPCH=""
	ABPCHSRC=""
	ABSRCS=""
	ABLIBOPTS=""
	ABDLLOPTS=""
	ABPREBUILDEVENT=""
	ABBUILDEVENT=""
	ABPOSTBUILDEVENT=""
	ABCLEANEVENT=""
}

#
#	autobuildCustomSetup 
#	Setup information for custom build
#
autobuildCustomSetup() {
	if [ -z ${INCLUDE+x} ]; then
		echo "You are running this bash script using MSYS but visual studio is missing."
		echo "You must open a \"Visual Studio .NET Command Prompt\" to run this script."
		return 1
	fi
	
	#set the ABCOMPILER and ABLINKER
	ABCOMPILER="cl"
	if [ $ABSTATIC ]; then
		ABLINKER="lib"
		ABLINKERDEFOPTS=" /nologo /nodefaultlib"
	else
		ABLINKER="link"
		ABLINKERDEFOPTS=" /nologo /DLL"
	fi
}

#
#	autobuildFixAutoPathing
#	Fix an issue with MSYS
#
autobuildFixAutoPathing() {
	ABPREPROCDEF=" ${ABPREPROCDEF}"
	ABCOMPILEOPTS=" ${ABCOMPILEOPTS}"
}

#
#	autobuildRunEvent 
#	Run an event and check for errors
#
autobuildRunEvent() {
	eval $1	
	if [ $ABBUILDFAILED ]; then exit 1; fi
}

#
#	autobuildCustomMSVC 
#	Custom build using msvc
#
autobuildCustomMSVC() {
	if [ ! $ABCOMPILER ]; then autobuildCustomSetup; fi
	
	(
		#pre-build event
		autobuildRunEvent "$ABPREBUILDEVENT"
		
		#build event
		autobuildRunEvent "$ABBUILDEVENT"
		
		#post-build event
		autobuildRunEvent "$ABPOSTBUILDEVENT"
	)
}

#
#	ABCheckModifiedFile 
#	Compare library file date to files in path
#
ABCheckModifiedFile() {
	local reference=""
	local folder="${ABSCPATH}${ABFOLDER}"
	
	if [ -z "$ABSRCS" ]; then
		folder=${folder}*
	else
		folder=${folder}$ABSRCS
	fi

	if [ $ABSTATIC ]; then
		reference="${ABLIBPATH}${ABLIBNAME}"
	else
		reference="${ABBINPATH}${ABDLLNAME}"
	fi
	
	for entry in $folder; do
		if [ -f "$entry" ]; then
			if [ "$reference" -ot "$entry" ]; then
				echo "1"
				return
			fi
		fi
	done

	echo "0"
}

#
#	autobuildBuild 
#	build function
#
autobuildBuild() {
	if [ $ABBUILDFAILED ]; then return 0; fi
	
	export ABLIBNAME="${ABPROJNAME}.lib"
	export ABDLLNAME="${ABPROJNAME}.dll"

	if [ ! $ABCLEAN ]; then
		local build=1
		
		if [ $ABSTATIC ] && [ -f "${ABLIBPATH}${ABLIBNAME}" ]; then
			build=$(ABCheckModifiedFile)
		elif [ ! $ABSTATIC ] && [ -f "${ABLIBPATH}${ABLIBNAME}" ] && [ -f "${ABBINPATH}${ABDLLNAME}" ]; then
			build=$(ABCheckModifiedFile)
		fi
		
		if [ $build == 1 ]; then
			if [ "$ABPLATFORM" = "MSYS" ]; then
				autobuildCustomMSVC
			else
				echo "No compiler tool available for current platform. Aborting..."
				exit 1
			fi
		else
			echo "Nothing to do for ${ABLABELNAME}"
		fi
	else
		eval $ABCLEANEVENT
	fi

	autobuildReset
}

#
#	autobuildPreBuildEvent
#	Events to execute before building
#
autobuildPreBuildEvent()
{
	echo "=== Building ${ABLABELNAME} ==="
	echo ""
	
	# create intermediate directory
	ABINTPATH="${ABINTPATH}${ABPROJNAME}/"
	mkdir -p "$ABINTPATH"
	
	autobuildFixAutoPathing
	
	cd ${ABSCPATH}${ABFOLDER}
}

#
#	autobuildPostBuildEvent
#	Events to execute after building
#
autobuildPostBuildEvent()
{
	echo ""
	echo "=== Successfully built ${ABLABELNAME} for ${ABPLATFORM[1]}/${ABARCH} ==="
}

#
#	autobuildCleandEvent
#	Default cleaning event
#
autobuildCleandEvent()
{
	echo -n "Cleaning ${ABLABELNAME}... "
	rm -r -f "${ABINTPATH}${ABPROJNAME}"
	rm -f "${ABLIBPATH}${ABLIBNAME}"
	rm -f "${ABBINPATH}${ABDLLNAME}"
	echo "Done"
}

#
#	autobuildCustomBuild 
#	default building switches
#
autobuildCustomBuild() {
	if [ $ABBUILDFAILED ]; then return 0; fi

	if [ ! $ABCLEAN ]; then
		ABCOMPILEOPTS="$ABCOMPILEOPTS /c /fp:precise"
		if [ $ABDEBUG ]; then
			if [ $ABSTATIC ]; then
				ABCOMPILEOPTS="$ABCOMPILEOPTS /MTd"
			else
				ABCOMPILEOPTS="$ABCOMPILEOPTS /MDd"
			fi
		
			ABPREPROCDEF="$ABPREPROCDEF /D _DEBUG /D DEBUG"
		else
			if [ $ABSTATIC ]; then
				ABCOMPILEOPTS="$ABCOMPILEOPTS /MT"
			else
				ABCOMPILEOPTS="$ABCOMPILEOPTS /MD"
			fi
			
			ABPREPROCDEF="$ABPREPROCDEF /D NDEBUG"
			ABLIBOPTS="$ABLIBOPTS /LTCG"
		fi
		
		if [ ABSDK71 == 1 ]; then
			ABPREPROCDEF="$ABPREPROCDEF /D _USING_V110_SDK71_"
		fi
		
		if [ ! $ABPREBUILDEVENT ]; then ABPREBUILDEVENT="autobuildPreBuildEvent"; fi
		if [ ! $ABBUILDEVENT ]; then ABBUILDEVENT="cmd.exe \/c \"${ABPATH}custom_build.bat\""; fi
		if [ ! $ABPOSTBUILDEVENT ]; then ABPOSTBUILDEVENT="autobuildPostBuildEvent"; fi
	else
		if [ ! $ABCLEANEVENT ]; then ABCLEANEVENT="autobuildCleandEvent"; fi
	fi
	
	ABCUSTOMBUILD=1

	autobuildBuild
}
