# party_seats.awk
# by Mark Tyler 2014-6-19

# examples:

#	cat 2010.tsv | awk -f party_seats.awk | sort -t $'\t' -rnk 7
# This counts up the total seat placings for each political party in the 2010 general election, sorting by total (i.e. 1st + 2nd + 3rd).



BEGIN {
	FS = "\t"
	ROW = 10
	PLACE_MAX = 5
}

NR>3 {
	if ( $1 )
	{
		ROW = 1
	}
	else if ( ROW > PLACE_MAX )
	{
		next
	}

	if ( $4 == "" )
	{
		# No more party placings for this seat
		ROW = PLACE_MAX + 1
		next
	}

	PLACES[ $4, ROW ] += 1

	ROW++
}

END {
	# Create list of parties
	for ( I in PLACES )
	{
		split ( I, J, SUBSEP )

		PARTIES[ J[1] ]
	}

	for ( PARTY in PARTIES )
	{
		TOT = 0

		printf "%s\t", PARTY

		for ( P = 1; P <= PLACE_MAX; P++ )
		{
			FREQ = PLACES[ PARTY, P ]
			TOT += FREQ

			if ( FREQ == "" )
			{
				FREQ="0"
			}

			printf "%s\t", FREQ
		}

		printf "%s\n", TOT
	}
}
