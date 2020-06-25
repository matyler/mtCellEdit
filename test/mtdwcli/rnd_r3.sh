#!/bin/bash
# Create an mtDWCLI script to process soda input files (r2 -> r3)
# M.Tyler 2019-2-12


ITEM_TOT=$1

for (( item=0; item<$ITEM_TOT; item++ ))
do
	printf "soda decode r2/%06i.soda r3/%06i\n" $item $item
done

