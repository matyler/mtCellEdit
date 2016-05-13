#!/bin/sh
# Package up parts of mtcelledit distribution into Debian .deb files
# by Mark Tyler, 2012-11-17


MT_flush_func()
{
	chmod a+w -R debian/*/tmp
	rm -vrf debian/*/tmp bcfile.txt
}


MT_build_app_func()
{
	DEBUG=$(echo "$CONF" | grep debug)

	# Exit on error
	set -e


	if [ "$DEBUG" != "" ]
	then
		echo DEBUGGING MODE
		echo

		export DEB_BUILD_OPTIONS="nostrip noopt"
	else
		export DEB_BUILD_OPTIONS=
	fi

	cd $CWD/..

	# Debian bureaucrats don't like underscores
	DEB_PKG=$(echo $PKG | sed -e "s/_/./g")
	DEB_SRC_DIR=$DEB_PKG\_$SRC_VER
	NEW_SRC_DIR=$DEB_PKG-$SRC_VER

	# Prepare the source code tarball & PKGBUILD
	echo BUILDING ...... $PKG from $SRC_DIR

	mkdir $CWD/debian/$PKG/tmp
	cp -ar $SRC_DIR $CWD/debian/$PKG/tmp/$NEW_SRC_DIR

	cd $CWD/debian/$PKG/tmp/
	tar czf $DEB_SRC_DIR.orig.tar.gz $NEW_SRC_DIR

	cd $NEW_SRC_DIR
	dh_make --single --copyright gpl3 \
		--email "marktyler@users.sourceforge.net"

	cp $CWD/debian/$PKG/mt/* debian

	PKG_CONF_FILE="debian/rules"
	mv $PKG_CONF_FILE $PKG_CONF_FILE.tmp
	cat $PKG_CONF_FILE.tmp |
		awk -v CF="$CONF" '{ sub ("@MT_CONF@", CF, $0); print }' \
		> $PKG_CONF_FILE

	PKG_CONF_FILE="debian/changelog"
	mv $PKG_CONF_FILE $PKG_CONF_FILE.tmp
	cat $PKG_CONF_FILE.tmp |
		awk	-v SA="$SRC_VER-1" \
			-v SB="$SRC_VER-$PKG_REL" \
			-v SC="$DEB_PKG" \
			'{ if ( $1==SC ) { sub (SA, SB, $0) } print }' \
		> $PKG_CONF_FILE

	debuild -us -uc
	cd ..

	dpkg -I *.deb
	dpkg -c *.deb

	read JUNK

	sudo dpkg -i *.deb


	# Don't exit on error
	set +e
}


MT_remove_func()
{
	DEB_PKG_LIST=$(echo $PKG_LIST | sed -e "s/_/./g")

	echo	sudo dpkg -P $DEB_PKG_LIST
		sudo dpkg -P $DEB_PKG_LIST
}


. ./_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Debian - DEB"

MT_ACTION_BUILD

