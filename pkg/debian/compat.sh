#!/bin/bash

# Sets the compat level for each package in the tree
# 2020-1-29 M.Tyler


for DIR in */mt
do
	FILENAME=$DIR/compat
	echo "9" > $FILENAME
done

