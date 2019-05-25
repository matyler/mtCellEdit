#!/bin/bash
# cedsheetbook.sh - Puts sheet files into a single book file
# usage: cedsheetbook.sh BOOKFILENAME SHEETFILE... | mtcedcli
# e.g. cedsheetbook.sh ~/output.zip *.tsv *.csv | mtcedcli
# by Mark Tyler 19th February 2013


echo "delete sheet"
echo "save as \"$1\""

shift

while [ "$1" != "" ]
do
	echo "import book \"$1\""
	shift
done

echo "save"
