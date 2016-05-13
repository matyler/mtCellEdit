#!/bin/sh
# Build and install script for all packages.
# Mark Tyler 2011-12-7


MT_flush_func()
{
	if [ "$PKG_LIST" = "" ]
	then
		DIR_LIST=$APPS_DIR_ALL
	else
		DIR_LIST=""

		for PKG in $PKG_LIST
		do
			# Find the directory for each package
			DIR_LIST="$DIR_LIST $(cat apps_list.tsv |
				awk -v "PKG=$PKG" '$1==PKG { print $2; exit }')"
		done

		# Sort DIR_LIST and remove duplicates
		DIR_LIST=$(echo $DIR_LIST |
			awk '{ for (i=1; i<=NF; i++) { print $i } }' |
			sort -u)
	fi

	for PKG in $DIR_LIST
	do
		UNDERLINE_TEXT "Flushing $PKG"

		cd $CWD/../$PKG
		./configure flush
	done

	rm -f bcfile.txt
}


MT_build_app_func()
{
	echo
	echo $PKG

	MT_GET_PKG_DIR "$PKG"
	cd $CWD/../$PKG_DIR

	MT_RUN_COM $CONF


	# On error exit
	set -e

	make clean
	make
	sudo make install

	# Don't exit on error
	set +e
}


MT_remove_func()
{
	# On error exit
	set -e


	REV_APPS=$( echo $PKG_LIST |
		awk '{ for (i=NF; i>0; i--) { printf $i" " } }' )

	echo
	echo $REV_APPS
	echo

	for PKG in $REV_APPS
	do
		UNDERLINE_TEXT "Removing $PKG"

		CONF=$(cat "$CWD/$BCFILE" |
			awk -F "\t" -v PKG="$PKG" '$1==PKG { print $2; exit }')

		MT_GET_PKG_DIR "$PKG"
		cd $CWD/../$PKG_DIR

		# Don't exit on error
		set +e

		./configure flush

		# On error exit
		set -e

		$CONF
		sudo make uninstall
	done


	# Don't exit on error
	set +e
}


. ./_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Generic - ./configure, make, make install"

MT_ACTION_BUILD

