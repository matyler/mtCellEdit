#!/bin/sh
# Update handbooks
# Mark Tyler 2017-8-7


# On error exit
set -e

cd ..


for DIR in */handbook
do
	cd $DIR/..
	./configure
	cd handbook
	make
	cd ..
	./configure flush
	cd ..
done

