#!/bin/bash
# Build script to create AppImages for all GUI apps.
# Mark Tyler 2018-3-23


# Exit on error
set -e


wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/7/linuxdeployqt-7-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage



#for PKG in "mtcrul-qt5" "mtcelledit-qt5" "mtdatawell-qt5" "mtpixy-qt5" "mtraft-qt5"
for PKG in "mtcelledit-qt5" "mtpixy-qt5"
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
