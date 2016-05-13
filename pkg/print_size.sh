#!/bin/sh
# Report size of codebase
# Mark Tyler 2013-2-8


# On error exit
set -e

. ./apps_list.txt

cd ..

APPS=$(echo $APPS | tr " " "\n" | sort)

for PKG in $APPS_DIR_ALL
do
	cd $PKG

	RES=$(./configure wc | tail -n 1 | awk '{ printf "%8s%8s", $1, $3 }')

	if [ "$RES" != "" ]
	then
		printf "%-15s%s\n" "$PKG" "$RES"
	fi

	cd ..
done

