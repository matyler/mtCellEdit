#!/bin/bash

# Build man pages for the web
# M.Tyler 2018-6-25


. ./build_core.txt


MANDIR=$OUTDIR/man1

mkdir -p $MANDIR


for PAGE in ../../*/man/*.1
do
	PKG=$(basename $PAGE)
#	PKG=$(echo "$PKG" | sed 's/\.1//g')
	echo "$PKG - $PAGE -> $OUTDIR"

	man2html -r -M "http://doc.marktyler.org/" $PAGE |
		awk 'BEGIN { OUT=0 } /DOCTYPE/ { OUT=1; } { if ( OUT ) {print;} }' > $MANDIR/$PKG.html

done

