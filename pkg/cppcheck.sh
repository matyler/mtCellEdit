#!/bin/sh
# Build script for all packages using cppcheck static analyzer.
# Mark Tyler 2013-2-1


# On error exit
#set -e

. ./apps_list.txt

cd ..

for PKG in $APPS_DIR_ALL
do
	echo
	echo $PKG
	echo

	cd $PKG
	cppcheck --enable=all --inconclusive .
	cd ..
done

