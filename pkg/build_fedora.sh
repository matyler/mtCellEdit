#!/bin/sh
# Package up parts of mtcelledit distribution into Fedora .rpm files
# by Mark Tyler, 2013-5-18


MT_flush_func()
{
	rm -rf $CWD/fedora/tmp/*/* bcfile.txt
}


MT_build_app_func()
{
	# Exit on error
	set -e


	SPECFILE=$CWD/fedora/tmp/SPECS/$PKG.spec

	# Prepare the source code tarball & PKGBUILD
	echo BUILDING ...... $PKG from $SRC_DIR

	cd $CWD/fedora/tmp/SOURCES/
	cp -ar $CWD/../$SRC_DIR $PKG-$SRC_VER
	tar czf $PKG-$SRC_VER.tar.gz $PKG-$SRC_VER

	# Set up SPEC file
	cat $CWD/fedora/spec/$PKG.spec |
		awk -v CF="$SRC_VER" '{ sub ("@MT_VERSION@", CF, $0); print }' |
		awk -v CF="$CONF" '{ sub ("@MT_CONF@", CF, $0); print }' |
		awk -v CF="$PKG_REL" '{ sub ("@MT_RELVER@", CF, $0); print }' \
		> $SPECFILE

	# Echo commands
	set -x

	rpmbuild --define "_topdir $CWD/fedora/tmp" -bb $SPECFILE

	RPMFILE=$CWD/fedora/tmp/RPMS/*/$PKG-$SRC_VER*.rpm

	rpm -qvpil $RPMFILE

	read JUNK

	sudo rpm -Uvh --replacepkgs $RPMFILE

	# Don't echo commands
	set +x


	# Don't exit on error
	set +e
}


MT_remove_func()
{
	sudo rpm -ev $PKG_LIST
}

. ./_build_global.txt



MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Fedora - RPM"

MT_ACTION_BUILD

