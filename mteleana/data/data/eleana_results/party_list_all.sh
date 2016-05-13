#!/bin/sh

# List all party names for each year.
# by Mark Tyler 2015-7-23

# Sort by name, then year:
# ./party_list_all.sh | cedsort -i - 1,ac,2,a -o -
# Sort by year, then name:
# ./party_list_all.sh | cedsort -i - 2,a,1,ac -o -
# Sort by name, without printing file:
# ./party_list_all.sh 1 | sort -u



func_normal()
{
	ls *.tsv |
	while read INPUT
	do
		cat $INPUT | awk -v "TEXT=$INPUT" -f party_list.awk
	done
}

func_simple()
{
	ls *.tsv |
	while read INPUT
	do
		cat $INPUT | awk -f party_list.awk
	done
}



case $1 in
1)	func_simple
	;;
*)	func_normal
	;;
esac
