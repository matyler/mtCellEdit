#!/bin/bash
# Build all PNG's from the SVG
# M.Tyler 2016.11.02


# Print commands
set x+

for i in $(seq -w 1 10)
do
	FILE=$i.png
	HEIGHT=$((96 * 10#$i))

	inkscape -z -e $FILE --export-height=$HEIGHT default.svg
done

