#!/bin/bash
# Build script to create AppImages for all GUI apps.
# Mark Tyler 2018-3-23


# Exit on error
set -e


CENTOS7="https://github.com/probonopd/linuxdeployqt/releases/download/7/linuxdeployqt-7-x86_64.AppImage"
MISC8="https://github.com/probonopd/linuxdeployqt/releases/download/8/linuxdeployqt-continuous-x86_64.AppImage"
DEBIAN11="https://github.com/probonopd/linuxdeployqt/releases/download/10/linuxdeployqt-continuous-x86_64.AppImage"
LATEST="https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"

DLFILE="$CENTOS7"
#DLFILE="$MISC8"
#DLFILE="$DEBIAN11"
#DLFILE="$LATEST"


CWD=$(pwd)
VERSION=$(cat $CWD/../VERSION)
PKGQT="qt5"
PKGNAMES="mtcelledit mtpixy mtdatawell mtcrul mtraft mtnush mtwasp"


echo CWD	= $CWD
echo VERSION	= $VERSION
echo APPS       = $PKGNAMES


wget -c "$DLFILE"
chmod a+x linuxdeployqt*.AppImage


for PKG in $PKGNAMES
do
	echo	./build_appimage.sh $PKG-$PKGQT
		./build_appimage.sh $PKG-$PKGQT
done

PKGFILES=$(ls mt*.AppImage | awk -F "-" '{print $1}')

for PKG in $PKGFILES
do
	echo	rename $PKG-$PKGQT $PKG-$VERSION-$PKGQT $PKG-$PKGQT*.AppImage
		rename $PKG-$PKGQT $PKG-$VERSION-$PKGQT $PKG-$PKGQT*.AppImage
done


echo
echo "Remove download??"
echo

read JUNK

if [ "$JUNK" != "" ]
then
	set -x		# Output commmands

	rm linuxdeployqt*.AppImage

	set +x		# DONT Output commmands
fi
