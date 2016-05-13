# list_seats.awk
# by Mark Tyler 2015-6-8
#
# example:
# cat 2010.tsv | awk -f list_seats_region.awk | sort
# sorts all of the seat names into alpha order.


BEGIN {
	FS = "\t"
}

NR>3 && $1 {
	printf "%s\t%s\t%s\n", $1, $6, $7
}
