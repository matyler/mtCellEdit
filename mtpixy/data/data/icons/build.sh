#!/bin/bash
# Build all PNG's from the SVG
# M.Tyler 2016.11.02


# Print commands
set x+

# FIXME - nothing to do as we only need the SVG file
exit 0

case $(inkscape --version) in

	"Inkscape 1"*)
		PNG_OPT="--export-filename"
		;;
	*)
		PNG_OPT="-z --export-png"
		;;
esac


for i in $(seq -w 1 10)
do
	FILE=$i.png
	HEIGHT=$((96 * 10#$i))
	WIDTH=$(($HEIGHT * 3 / 2))

	inkscape $PNG_OPT=$FILE --export-width=$WIDTH --export-height=$HEIGHT \
		default.svg
done

