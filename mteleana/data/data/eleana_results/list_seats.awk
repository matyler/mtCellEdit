# list_seats.awk
# by Mark Tyler 2015-6-8
#
# example:
# cat 2010.tsv | awk -f list_seats.awk | sort
# sorts all of the seat names into alpha order.


BEGIN {
	FS = "\t"
}

NR>3 && $1 {
	printf "%s\n", $1
}
