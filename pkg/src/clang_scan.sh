#!/bin/bash
# Build script for all packages using clang's static analyzer.
# Mark Tyler 2012-5-19


# On error exit
set -e

. ./src/apps_list.txt

cd ..

# On error DONT exit
set +e


for NAME in scan-build scan-build-4.0 scan-build-5.0 scan-build-6.0 scan-build-7.0
do
	BIN=$(which $NAME)

	if [ "$BIN" != "" ]
	then
		break
	fi
done

# On error exit
set -e


if [ "$BIN" = "" ]
then
	echo
	echo Unable to find scan-build
	echo

	exit
fi

echo
echo "BIN = $BIN"
echo


for PKG in $APPS_DIR_ALL
do
	cd $PKG
	$BIN ./configure
	$BIN make
	cd ..
done

