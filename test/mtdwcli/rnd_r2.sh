#!/bin/bash
# Create an mtDWCLI script to process raw input files (r1 -> r2)
# M.Tyler 2019-2-12


echo "butt add otp rndfiles_1"
echo "butt add buckets 5"

ITEM_TOT=$1

for (( item=0; item<$ITEM_TOT; item++ ))
do
	printf "soda encode r1/%06i r2/%06i.soda\n" $item $item
done

