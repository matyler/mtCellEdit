#!/bin/bash
# Builds the source tarball
# M.Tyler 2010-3-11, revised 2013-2-15, 2014-10-15 to be generic for:
#	PACKAGENAME-VERSION.DATE/pkg
#	VERSION	= MAJOR.MINOR
#	DATE	= YYYY.MMDD.HHMM


./build_install.sh flush

./update_mtconf.sh


# On error exit
set -e

cd ..
CWD=$(pwd)


CHECK_EMPTY()
{
	# Check for empty directories: git doesn't track these
	cd $CWD
	EMPTY=$(find . -not -path ./.git -prune -type d -empty)

	if [ "$EMPTY" != "" ]
	then
		echo
		echo "Error! Empty directories found"
		echo
		echo $EMPTY

		exit
	fi

	SMALL=$(find . -type f -size -3c)
	if [ "$SMALL" != "" ]
	then
		echo
		echo "WARNING! Small files less than 3 bytes:"
		echo
		echo $SMALL

		read JUNK
	fi
}

CHECK_HIDDEN()
{
	# Check for hidden files & directories
	cd $CWD
	HIDDEN_FILES=$(find . -not -path ./.git -prune -type f -iname ".*")
	HIDDEN_DIRS=$(find . -not -path ./.git -prune -type d -iname ".*")
	if [ "$HIDDEN_FILES$HIDDEN_DIRS" != "." ]
	then
		echo
		echo "Error! Hidden files & directories found"
		echo
		echo $HIDDEN_FILES
		echo $HIDDEN_DIRS

		exit
	fi
}

cd $CWD
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

# Clear any test data
cd $CWD/test
./configure flush

# Final checks
CHECK_EMPTY
CHECK_HIDDEN

# Finally build the tarball
cd $CWD/..
chmod -R -77 $PACKAGE
chmod -R a+r $PACKAGE

tar cJf ~/$PACKAGE.tar.xz --owner=root --group=root --exclude $PACKAGE/.git $PACKAGE

echo

