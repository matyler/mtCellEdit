# Build index.html
# M.Tyler 2018-6-25


{
	if ( $0 == "LIST_OF_APPS_WITH_LINKS" )
	{
		system ( "cat tmp/app.txt" )
	}
	else if ( $0 == "LIST_OF_MAN_PAGES_WITH_LINKS" )
	{
		system ( "cat tmp/man.txt" )
	}
	else
	{
		print
	}
}
