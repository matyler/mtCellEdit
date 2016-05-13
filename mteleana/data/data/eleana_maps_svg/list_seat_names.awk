# list_seat_names.awk
# by Mark Tyler 2015-6-8
#
# example:
# cat 2010.svg | awk -f list_seat_names.awk | cut -d '"' -f2 | sort
# sorts all of the seat names into alpha order.


BEGIN {
	FS = "\t"
}

$0 ~ "<path" {
	MODE = "path"
}

$0 ~ "id" && MODE == "path" {
	print "ID =" $0
	MODE = "id"
}
