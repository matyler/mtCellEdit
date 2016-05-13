# region_votes.awk
# by Mark Tyler 2015-4-3
#
# examples:
#
#	cat 2010.tsv | awk -f region_votes.awk | cedsort -i - "1,ac,3,d" -o -
#
# This counts up the total votes for each party in each of the regions.



BEGIN {
	FS = "\t"
}

NR>3 {
	if ( $7 )
	{
		REGION = $7
	}

	if ( $4 )
	{
		PLACES[ $4, REGION ] += $5
	}
}

END {
	for ( I in PLACES )
	{
		split ( I, J, SUBSEP )
		printf "%s\t%s\t%s\n", J[2], J[1], PLACES[ I ]
	}
}
