# party_totals.awk
# by Mark Tyler 6th December 2010
# Tweaked 22nd August 2011 to be ~25% faster (let sed remove ' at end)

# examples:

#	cat 2010.tsv | awk -f party_totals.awk | sort | sed -e "s/^'//"
# This counts up the totals for each political party in the 2010 general election, sorting by name of party.

#	cat 2010.tsv | awk -f party_totals.awk | sort -t $'\t' -rnk 2 | head | sed -e "s/^'//"
# This outputs a top 10 of the most popular political parties from the 2010 general election.


BEGIN {
	FS = "\t"
}

NR>3 {
	if ( $5 > 0 ) list[ $4 ] += $5
}

END {
	for ( party in list )
	{
		printf "%s\t%s\n", party, list[ party ]
	}
}
