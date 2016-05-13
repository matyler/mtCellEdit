#!/bin/sh
# Package up parts of mtcelledit distribution into Arch XZ files
# by Mark Tyler, 2011-9-22


MT_flush_func()
{
	chmod a+w -R arch/*/*.tar.gz \
		arch/*/PKGBUILD \
		arch/*/pkg \
		arch/*/src \
		arch/*/*.pkg.tar.xz
	rm -vrf arch/*/*.tar.gz \
		arch/*/PKGBUILD \
		arch/*/pkg \
		arch/*/src \
		arch/*/*.pkg.tar.xz \
		bcfile.txt
}


MT_build_app_func()
{
	# Exit on error
	set -e


	# Prepare the source code tarball & PKGBUILD
	echo BUILDING ...... $PKG from $SRC_DIR

	cd $CWD/arch/$PKG/
	cp -ar $CWD/../$SRC_DIR $PKG-$SRC_VER
	tar czf $PKG-$SRC_VER.tar.gz $PKG-$SRC_VER
	rm -rf $PKG-$SRC_VER
	CHECKSUM=$(md5sum $CWD/arch/$PKG/$PKG-$SRC_VER.tar.gz |
		awk '{print $1}')

	cat $CWD/arch/$PKG/PKGBUILD.txt |
		sed -e "s/^pkgver$/pkgver=$SRC_VER/" |
		sed -e "s/^md5sums$/md5sums=\('$CHECKSUM'\)/" |
		awk -v CF="$CONF" '{ sub ("@MT_CONF@", CF, $0); print }' |
		awk -v CF="$PKG_REL" '{ sub ("@MT_RELVER@", CF, $0); print }' \
		> $CWD/arch/$PKG/PKGBUILD


	# Build & install package
	cd $CWD/arch/$PKG
	LC_ALL=C

	case "$CONF" in
	*"debug"* )
		# Building with debugging so don't strip any binaries
		cp /etc/makepkg.conf .
		echo "OPTIONS+=(debug !strip)" >> makepkg.conf
		MAKEPKG_CONF=makepkg.conf makepkg -A
		;;
	* )
		makepkg -A
		;;
	esac

	PKGFILE=$PKG*.pkg.tar.xz

	echo
	echo $PKGFILE

	echo
	ls -l $PKGFILE

	echo
	tar tvf $PKGFILE

	echo
	sudo pacman -U $PKGFILE


	# Don't exit on error
	set +e
}


MT_remove_func()
{
	echo	sudo pacman -R $PKG_LIST
		sudo pacman -R $PKG_LIST
}


. ./_build_global.txt


MT_STARTUP

MT_PARSE_ARGS "$@"

MT_CREATE_BCFILE

MT_DISPLAY_INFO "Arch - PKG"

MT_ACTION_BUILD

