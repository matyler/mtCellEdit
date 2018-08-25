#!/bin/sh
# Update man pages in each package
# Mark Tyler 2017-8-7


# On error exit
set -e

cd ..
CWD=$(pwd)


# Remove old man pages
for DIR in */man
do
	cd $DIR
	rm -f *.1
	cd $CWD
done


BUILD_MAN_PAGE()
{
	echo "Man page - $1 $2"

	cd $CWD/$1
	./configure $2
	cd man
	make
	cd ..
	./configure flush
	cd $CWD
}

FIND_MAN_PAGES()
{
	if [ "$1" != "" ]
	then
		# Targetted man page
		ARGS="--use-$1"
		GREP="MTCF_ENABLE_QT"
	else
		# General man page
		ARGS=""
		GREP=""
	fi

	for CONF in */configure
	do
		MATCH=$(grep -l "$GREP" "$CONF")

		if [ "$MATCH" = "" ]
		then
			continue
		fi

		DIR=$(dirname $CONF)
		DIR=$(basename $DIR)

		if [ -d $DIR/man ]
		then
			BUILD_MAN_PAGE "$DIR" "$ARGS"
		fi
	done
}


# On error DON'T exit
set +e


# Update default man pages
FIND_MAN_PAGES

# Update Qt4 man pages
FIND_MAN_PAGES "qt4"

