# party_list.awk
# by Mark Tyler 2015-7-23

# examples:

#	cat 2010.tsv | awk -f party_list.awk | sort
# List all parties in alpha order from the 2010 election.


BEGIN {
	FS = "\t"
}

NR>3 && $4 {
	list[ $4 ]
}

END {
	for ( party in list )
	{
		printf "%s\t%s\n", party, TEXT
	}
}
