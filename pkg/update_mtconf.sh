#!/bin/sh
# Update core files to each package
# Mark Tyler 2013-2-8


# On error exit
set -e

. ./apps_list.txt

MTCONF=mtConf.txt


for PKG in test $APPS_DIR_ALL
do
	echo	cp -a $MTCONF ../$PKG
		cp -av $MTCONF ../$PKG
	echo
done


for DIR in ../*/desktop/
do
	echo	cp -a builddesktop.sh $DIR
		cp -av builddesktop.sh $DIR
	echo
done


for DIR in ../*/man/
do
	echo	cp -a buildman.sh $DIR
		cp -av buildman.sh $DIR
	echo
done

