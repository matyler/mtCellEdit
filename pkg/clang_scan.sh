#!/bin/sh
# Build script for all packages using clang's static analyzer.
# Mark Tyler 2012-5-19


# On error exit
set -e

. ./apps_list.txt

cd ..

for PKG in $APPS_DIR_ALL
do
	cd $PKG
	scan-build ./configure
	scan-build make
	cd ..
done

