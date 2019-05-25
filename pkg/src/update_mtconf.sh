#!/bin/bash
# Update core files to each package
# Mark Tyler 2013-2-8


# On error exit
set -e


. ./src/apps_list.txt


MTCONF=src/mtConf.txt
MTDESK=src/builddesktop.sh
MTMAN=src/buildman.sh
MTPY=src/buildpy.sh


for PKG in test $APPS_DIR_ALL
do
	echo	cp -av $MTCONF ../$PKG
		cp -av $MTCONF ../$PKG
	echo
done


for DIR in ../*/desktop/
do
	echo	cp -av $MTDESK $DIR
		cp -av $MTDESK $DIR
	echo
done


for DIR in ../*/man/
do
	echo	cp -av $MTMAN $DIR
		cp -av $MTMAN $DIR
	echo
done


for DIR in $(dirname ../*/src/*.py | sort -u)
do
	echo	cp -av $MTPY $DIR
		cp -av $MTPY $DIR
	echo
done

