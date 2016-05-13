# val_places.awk
# by Mark Tyler 2014-8-9

# examples:

#	cat 2010.tsv | awk -f val_places.awk
# Count how many seats have a 1st, 2nd, 3rd, etc placed candidate.



BEGIN {
	FS = "\t"
	PLACE_MAX = 5
	ROW = PLACE_MAX + 1
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

	PLACES[ ROW ] ++
	ROW ++
}

END {
	for ( P = 1; P <= PLACE_MAX; P++ )
	{
		printf "%s - %s\n", P, PLACES[ P ]
	}

	printf "\n\n"
}
