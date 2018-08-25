#!/bin/bash

# Build root HTML page for the web
# M.Tyler 2018-6-25


. ./build_core.txt


cp -ar raw/* $OUTDIR


APP_FILE=$CWD/tmp/app.txt
MAN_FILE=$CWD/tmp/man.txt

> $APP_FILE
> $MAN_FILE

# Apps

APP_FILE_SUFFIX=en_GB/index.html

for HTML in $OUTDIR/*/$APP_FILE_SUFFIX
do
	DIR=$(echo "$HTML" | sed 's/\/en_GB\/index\.html$//g')
	PKG=$(basename $DIR)

	echo "<a href=\"$PKG/$APP_FILE_SUFFIX\">$PKG</a><BR>" >> $APP_FILE
done


# Man pages

for PAGE in $OUTDIR/man1/*.1.html
do
	DIR=$(echo "$PAGE" | sed 's/\.1\.html$//g')
	PKG=$(basename $DIR)

	echo "<a href=\"man1/$PKG.1.html\">$PKG</a><BR>" >> $MAN_FILE
done

cat $APP_FILE $MAN_FILE

cat $OUTDIR/index_skeleton.html |
	awk -f build_html.awk > $OUTDIR/index.html

