#!/bin/sh
# 2016.08.20 by M.Tyler
# Create a bunch of partial files



create_part()
{
	FILENAME="part_$1_$2"
	echo Creating $FILENAME

	dd if=$2 of=$FILENAME bs=$1 count=1
}


rm part_*
> empty


ls _*.bmp _*.gif _*.png _*.jpg |
while read line
do
	create_part "20" "$line"
	create_part "400" "$line"
done


ls -l

