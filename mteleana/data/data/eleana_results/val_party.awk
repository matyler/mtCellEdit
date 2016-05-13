# val_party.awk
# by Mark Tyler 2014-8-8

# examples:

#	cat 2010.tsv | awk -f val_party.awk
# Validate that each result has a party & candidate attached to it.



BEGIN {
	FS = "\t"
	ROW = 0
}

NR>3 {
	if ( $1 )
	{
		ROW = 1
	}

	if ( $5 == "" )
	{
		ROW = 0
	}

	if ( ROW == 0 )
	{
		next
	}

	if ( $3 == "" )
	{
		printf "Error row %s - No Candidate\n", NR
		print $0
	}

	if ( $4 == "" )
	{
		printf "Error row %s - No Party\n", NR
		print $0
	}
}

END {
}
