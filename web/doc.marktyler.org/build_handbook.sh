#!/bin/bash

# Build handbook pages for the web
# M.Tyler 2018-6-25


. ./build_core.txt


TMPDIR=$CWD/tmp

for DIR in ../../*/handbook
do
	PKG=$(basename $(dirname $DIR))
	echo "$PKG"

	cd $CWD
	mkdir -p $OUTDIR/$PKG

	cd $DIR/..
	./configure --datadir="$TMPDIR"

	cd handbook
	make
	make install

	cd $CWD
	echo	cp -ar $TMPDIR/doc/$PKG*/* $OUTDIR/$PKG
		cp -ar $TMPDIR/doc/$PKG*/* $OUTDIR/$PKG
done

