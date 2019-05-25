#!/bin/bash
# cedbooksplit.sh - Extracts the sheets from a book
# usage: cedbooksplit.sh BOOKFILENAME OUPUTDIRECTORY
# e.g. cedbooksplit.sh ~/output.zip ~/temp_sheet
# by Mark Tyler 19th February 2013

# On error exit
set -e

mkdir -p "$2"

TMP_FILE="$2"/tmp.txt

printf "load \"$1\"\n" > "$TMP_FILE"

printf "load \"$1\"\nlist sheets" |
mtcedcli -q |
awk '!/^$/ && !/^mtcedcli/' |
while read SHEET
do
	LOWER=$(echo "$SHEET" | tr '[:upper:]' '[:lower:]')

	case "$LOWER" in
	*csv )	TYPE=csv;;
	* )	TYPE=tsv;;
	esac

	echo set sheet \"$SHEET\" >> "$TMP_FILE"
	echo export sheet \"$2/$SHEET\" $TYPE >> "$TMP_FILE"
done

cat "$TMP_FILE"

echo
echo Type y and press ENTER to run this script

read KEY

if [ "$KEY" = "y" ]
then
	cat "$TMP_FILE" | mtcedcli
fi

echo
