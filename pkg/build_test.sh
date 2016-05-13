#!/bin/sh
# Test Build.
# Mark Tyler 2014-4-13


MT_flush_func()
{
	echo "NOT FLUSHING!"
}


MT_build_app_func()
{
	echo
	UNDERLINE_TEXT "$PKG - Press ENTER to build"

	read JUNK

	MT_GET_PKG_DIR "$PKG"
	cd $CWD/../$PKG_DIR

	MT_RUN_COM $CONF


	# On error exit
#	set -e

	make clean
	make

	# Don't exit on error
#	set +e
}


MT_remove_func()
{
	echo "NOT REMOVING!"
}


. ./_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Build Test - ./configure, make"

MT_ACTION_BUILD

