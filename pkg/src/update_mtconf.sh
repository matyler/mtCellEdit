#!/bin/bash
# Update core files to each package
# Mark Tyler 2013-2-8 & 2021-6-11


CWD="$(pwd)"


# On error exit
set -e

. ./src/apps_list.txt


SCAN_FILES()
{
	GLOB=$1
	SRC=$2

	if ls $GLOB 1> /dev/null 2>&1; then
#		echo "files/dirs do exist: $GLOB"

		FILES=$(ls $GLOB)

		echo "FILES=$FILES"

		for DIR in $(dirname $FILES | sort -u)
		do
			echo	cp -av $SRC $DIR
				cp -av $SRC $DIR
			echo
		done
#	else
#		echo "files/dirs do not exist: $GLOB"
	fi

	echo
}


SCAN_DIRS()
{
	GLOB=$1
	SRC=$2

	for DIR in $GLOB
	do
		if [[ -d "$DIR" ]]; then
#			echo "dir does exist: $DIR"

			echo	cp -av $SRC $DIR
				cp -av $SRC $DIR
			echo
#		else
#			echo "dir does not exist: $DIR"
		fi
	done

	echo
}


cd $CWD/..

SCAN_DIRS	"test $APPS_DIR_ALL"	"src/mtConf.txt"
SCAN_DIRS	"*/desktop/"		"src/builddesktop.sh"
SCAN_DIRS	"*/man/"		"src/buildman.sh"
SCAN_FILES	"*/src/*.py"		"src/buildpy.sh"


