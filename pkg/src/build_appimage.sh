#!/bin/bash
# Build and install script for AppImage.
# Mark Tyler 2018-3-22


. ./src/build_generic.sh

SUDO=""
MAKE_ARGS="-j 8"

. ./src/_build_global.txt


MT_STARTUP


# All packages need these
MT_PARSE_ARGS libmtkit libmtpixy libmtqex5


# NOTE: we use *-qt5 & qex5, but this Qt reference is irrelevant until we set CONF below.

case "$1" in
"mtcelledit"* )	MT_PARSE_ARGS libmtcelledit mtcelledit-qt5;;
"mtdatawell"* )	MT_PARSE_ARGS libmtdatawell mtdatawell-qt5;;
"mtpixy"* )	MT_PARSE_ARGS mtpixy-qt5;;
"mtraft"* )	MT_PARSE_ARGS libmtcelledit mtraft-qt5;;
"mtcrul"* )	MT_PARSE_ARGS mtcrul-qt5;;
* )
	echo
	echo "ERROR - argument '$1' not recognised"
	echo
	exit 1
	;;
esac


case "$1" in
*"-qt5" ) CONF="--use-qt5";;
*"-qt6" ) CONF="--use-qt6";;
esac


APPDIR=$(pwd)/appdir

rm -rf $APPDIR
./build_install.sh flush

PRECONF="CFLAGS=\"-I$APPDIR/usr/include -O1 -std=gnu11\""
PRECONF="$PRECONF CXXFLAGS=\"-I$APPDIR/usr/include -O1 -std=gnu++11\""
PRECONF="$PRECONF LDFLAGS=\"-L$APPDIR/usr/lib -Wl,--as-needed\""
PRECONF="$PRECONF APPIMAGE_PREFIX=\"../..\""

CONF="$CONF --disable-man"

DESTDIR="$APPDIR"


MT_CREATE_BCFILE

UNDERLINE_TEXT "$1"

MT_DISPLAY_INFO "AppImage - ./configure, make, make install"

MT_ACTION_BUILD


# Remove files not needed & final tweaks
rm -rf $APPDIR/usr/include
rm -rf $APPDIR/usr/lib/libmt*.so
chmod a-x $APPDIR/usr/lib/libmt*


cd $APPDIR/..

./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -bundle-non-qt-libs
./linuxdeployqt*.AppImage ./appdir/usr/share/applications/*.desktop -appimage

