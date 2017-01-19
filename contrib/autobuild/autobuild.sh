#!/bin/bash
#
#	AutoBuild 
#
#	Automated build
#

#
#	usage 
#	Display autobuild script usage
#
usage() { 
	echo "Usage: autobuild.sh: [-c] [-d] [-s] LIBSCRIPT BINPATH LIBPATH INTPATH"
}

#
#	errorHandler 
#	Handle build errors
#
errorHandler() {
	echo "*******************************************************"
	echo "*** ${ABOP} FAILED -- Please check the error messages ***"
	echo "*******************************************************"
	read -n1 -r -p "Press a key to continue..." key
	exit 1
}

#
#
#
#
fixPath() {
	if [ -f "$1" ]; then echo "$1"; return 0; fi

	local LEN=${#1}-1

	if [ "${1:LEN}" != "/" ]; then
		echo "${1}/"
	else
		echo "$1"
	fi
}

#
#	relativeToAbsolute 
#	Convert an input path to an absolute one
#
relativeToAbsolute() {
	if [[ "$1" = /* ]]; then echo "$1"; fi
	
	echo "${PWD}/$1"
}

#
#	pathDir
#	return the directory part of the path
#
pathDir() {
	local PATH="${1%/*}/"
	
	# windows special case
	if [ "$PATH" = "$1/" ]; then PATH="${0%\\*}\\"; fi
	
	echo "$PATH"
}

#
#	detectOS
#	return the system running, the os name and the architecture
#
detectOS() {
	MACHINE_TYPE="$(uname -m)"
	if [ ! "${MACHINE_TYPE}" = "x86_64" ]; then
		MACHINE_TYPE="x86"
	fi

	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		echo "LINUX Linux ${MACHINE_TYPE}"
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "MACOS MacOS ${MACHINE_TYPE}"
	elif [[ "$OSTYPE" == "cygwin" ]]; then
		echo "CYGWIN WINDOWS ${MACHINE_TYPE}"
	elif [[ "$OSTYPE" == "msys" ]]; then
		echo "MSYS WINDOWS ${MACHINE_TYPE}"
	elif [[ "$OSTYPE" == "win32" ]]; then
		echo "WIN32 WINDOWS ${MACHINE_TYPE}"
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		echo "FREEBSD FreeBSD ${MACHINE_TYPE}"
	else
		echo "UNKNOWN"
	fi
}

#
#	pushVSPathToFront
#	pushes paths belonging to visual studio to the front
#
pushVSPathToFront() {
	local NEWPATH=""
	local VSPATHS=""
	IFS=$':'
	for p in $PATH; do
		if [ -z "$NEWPATH" ]; then
			NEWPATH="$p"
		elif [[ "$p" == *"Visual Studio"* ]]; then
			if [ -z "$VSPATHS" ]; then
				VSPATHS="$p"
			else
				VSPATHS="${VSPATHS}:${p}"
			fi
		else
			NEWPATH="$NEWPATH:${p}"
		fi
	done
	unset IFS
	echo "${VSPATHS}:${NEWPATH}"
}

#
#	isSDKDefinitionRequired
#	check for windows if we need _USING_V_SDK71_ special definition
#
isSDKDefinitionRequired() {
	if [[ -z "${LIB:=foo}" ]]; then
		if [[ "$LIB" == *"v7.1A"*  ]]; then
			if [[ "$LIB" == *"Visual Studio 14"*  ]]; then
				echo "1"
				return
			elif [[ "$LIB" == *"Visual Studio 12"*  ]]; then
				echo "1"
				return
			elif [[ "$LIB" == *"Visual Studio 12"*  ]]; then
				echo "1"
				return
			fi
		fi
	fi
	echo "0"
}

#
#	find build architecture
#	find which build architecture is used
#
findBuildArch() {
	if [[ "${ABPLATFORM[1]}" == "WINDOWS" ]]; then
		IFS=$':'
		for p in $PATH; do
			if [[ "$p" == *"Visual Studio"*"bin"* ]]; then
				if [[ "$p" == *"64"* ]]; then
					echo "x86_64"
				else
					echo "x86"
				fi
				break
			fi
		done
		unset IFS
	else
		echo "${ABPLATFORM[2]}"
	fi
}

#
#   run
#   run autobuild process
#
run() {
	echo
	echo "*******************************"
	echo "*** ${ABOP} Operation Started ***"
	echo "*******************************"
	echo

	# build
	. $ABSCRIPT
	if [ $ABBUILDFAILED ]; then false; fi

	echo
	echo "*********************************"
	echo "*** ${ABOP} Operation Completed ***"
	echo "*********************************"
	echo
}

#
#	autoBuild 
#	main Function
#
autoBuild() {
	#
	# autobuild switches
	#
	
	export ABDEBUG=""
	export ABSTATIC=""
	export ABCLEAN=""
	
	
	#
	# Check inputs
	#
	local OPTIND=1
	export OPTARG=""
	while getopts ":cds" opt; do
	  case $opt in
		c)
		  ABCLEAN=1
		  ;;
		d)
		  ABDEBUG=1
		  ;;
		s)
		  ABSTATIC=1
		  ;;
		*)
		  usage
		  exit 1
		  ;;
	  esac
	done
	shift $((OPTIND - 1))
	
	if [ $# -lt 4 ]; then
		echo "Wrong number of arguments."
		usage
		exit 1
	fi
	
	#
	# autobuild parameters
	#
	
	export ABSCRIPT=$(relativeToAbsolute $(fixPath $1))
	export ABBINPATH=$(relativeToAbsolute $(fixPath $2))
	export ABLIBPATH=$(relativeToAbsolute $(fixPath $3))
	export ABINTPATH=$(relativeToAbsolute $(fixPath $4))
	
	export ABPATH=$(pathDir $0)
	export ABSCPATH=$(pathDir $ABSCRIPT)
	export ABPLATFORM=($(detectOS))
	
	export ABARCH=$(findBuildArch)
	
	export ABBUILDFAILED=""
	
	export ABSDK71="0"
	
	local ABOP=""
	if [ $ABCLEAN ]; then
		ABOP="Clean"
	else
		ABOP="Build"
		
		#ensure that output folder have been created
		mkdir -p "$ABBINPATH"
		mkdir -p "$ABLIBPATH"
		mkdir -p "$ABINTPATH"
	fi
	
	#
	# windows need special things
	#
	if [[ "${ABPLATFORM[1]}" == "WINDOWS" ]]; then
		#
		# fix path
		#
		PATH=$(pushVSPathToFront)
		
		#
		# check for SDK7.1
		#
		ABSDK71=$(isSDKDefinitionRequired)
	fi
	
	#
	# load build tool
	#
	. ${ABPATH}contrib_build.sh
	
	autobuildReset
	
	run 2>&1 | tee ${ABINTPATH}autobuild.log
}

#
# set error handling
#
set -o pipefail  # trace ERR through pipes
set -o errtrace  # trace ERR through 'time command' and other functions
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value

trap errorHandler ERR

autoBuild $@
