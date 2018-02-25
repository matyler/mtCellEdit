#!/bin/sh
# Builds the source tarball
# M.Tyler 2010-3-11, revised 2013-2-15, 2014-10-15 to be generic for:
#	PACKAGENAME-VERSION.DATE/pkg
#	VERSION	= MAJOR.MINOR
#	DATE	= YYYY.MMDD.HHMM


./build_install.sh flush

./update_mtconf.sh
./update_handbook.sh

rm bcfile.txt


# On error exit
set -e

cd ..
CWD=$(pwd)
PACKAGE=$(basename $CWD)

echo $PACKAGE

PKG_NAME=$(echo $PACKAGE | awk -F "-" '{ print $1 }')
PKG_VER=$(cat $CWD/VERSION)
PKG_VER=$(echo $PKG_VER | awk -F "." '{ print $1 "." $2 }')

case "$1" in
"" )
	DATE="."$(date +%Y.%m%d.%H%M);;
* )
	DATE=;;
esac

NEW_PACKAGE=$PKG_NAME-$PKG_VER$DATE

echo "$PKG_VER$DATE" > $CWD/VERSION

cd ..

if [ $PACKAGE != $NEW_PACKAGE ]
then
	echo OLD=$PACKAGE
	echo NEW=$NEW_PACKAGE
	echo

	mv $PACKAGE $NEW_PACKAGE
	PACKAGE=$NEW_PACKAGE
fi

cd $PACKAGE
CWD=$(pwd)

# Update man pages with new version numbers
cd pkg
./update_man.sh

# Clear any test data
cd $CWD/test
./configure flush


# Finally build the tarball
cd $CWD/..
chmod -R -77 $PACKAGE
chmod -R a+r $PACKAGE

tar cJf ~/$PACKAGE.tar.xz --owner=root --group=root $PACKAGE

echo

