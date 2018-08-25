#!/bin/sh
# Build script to create AppImages for all GUI apps.
# Mark Tyler 2018-3-23


wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage


MODULES="mtcelledit-qt4"


for PKG in $MODULES
do
	echo	./build_appimage.sh $PKG
		./build_appimage.sh $PKG
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
