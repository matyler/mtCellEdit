This document outlines my testing procedures before this release.

* You cannot be root.  You must be a normal user with sudo rights.

* Your system must satisfy all of the requirements outlined in section 3 of the handbook.


Test Systems:
	DEB
		Ubuntu 21.04 (amd64) [x86_64] (2021)
	RPM
		CentOS 7.9 XFCE [x86_64] (2014-2021)		+ AppImage
		AlmaLinux 8.4 [x86_64] (2019-2021)
		Fedora 34 XFCE [x86_64] (2021)
		openSUSE 15.3 XFCE [x86_64] (2021)
	Arch
		Arco 20.11.9 [x86_64] (2020-2021)
	Local install
		Salix 14.2 [x86_32] (2016)
	bcfile install
		Raspberry Pi OS (Debian 10) [ARM_32] (2019-2020)


------------------------------------
Ubuntu 21.04 (amd64) [x86_64] (2021)
------------------------------------

Package deps:
sudo apt-get install dh-make pbuilder clang clang-tools valgrind bison flex cppcheck txt2tags automake libreadline-dev qtbase5-dev libsdl2-dev qtcreator time libpango1.0-dev librsvg2-dev libpng-dev libgif-dev libjpeg-dev libsqlite3-dev libsndfile1-dev inkscape mesa-common-dev libgl1-mesa-dev

* Install
	./build_debian.sh --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu" clean

* Build using clang-static to expose possible errors:
	./clang_scan.sh
	firefox /tmp/scan-build-*/index.html
	./build_install.sh flush

* Use cppcheck to expose programmer errors and unused functions:
	./cppcheck.sh > ~/cppcheck.txt 2>&1
	cat ~/cppcheck.txt | grep "^\[" | sort | wc
	cat ~/cppcheck.txt | grep "^\[" | sort | less
	cat ~/cppcheck.txt | less
	rm ~/cppcheck.txt

* Run ./test/* test suites to expose regressions.
	./configure debug
	make valg
	make clean
	make time
	./configure flush

* Use file format test suites from:
	- http://www.schaik.com/pngsuite/
	- http://entropymine.com/jason/bmpsuite/
	- https://code.google.com/archive/p/imagetestsuite/

	valgrind --leak-check=full --show-possibly-lost=no pixyls .../*.bmp
	valgrind --leak-check=full --show-possibly-lost=no pixyls .../*.gif
	valgrind --leak-check=full --show-possibly-lost=no pixyls .../*.jpg
	valgrind --leak-check=full --show-possibly-lost=no pixyls .../*.png

* Smoke test the apps.

* Skim documentation to expose cruft and mistakes.

* Remove
	./build_debian.sh remove clean


------------------------------
Fedora 34 XFCE [x86_64] (2021)
------------------------------

Package deps:
sudo yum install clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qt5-qtsvg qt6-qtbase-devel SDL2-devel cairo-devel pango-devel librsvg2-devel libpng-devel libjpeg-turbo-devel giflib-devel libsqlite3x-devel libsndfile-devel inkscape mesa-libGL-devel libubsan

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64 debug" clean

* Smoke test the apps.

* Remove
	./build_fedora.sh remove clean


------------------------------------
CentOS 7.9 XFCE [x86_64] (2014-2021)
------------------------------------

Package deps:
sudo yum install clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qt5-qtsvg SDL2-devel cairo-devel pango-devel librsvg2-devel libpng-devel libjpeg-turbo-devel giflib-devel libsqlite3x-devel libsndfile-devel inkscape mesa-libGL-devel

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64" clean

* Smoke test the apps.

* Remove
	./build_fedora.sh remove clean

* Build AppImages
	./build_appimage_all.sh


----------------------------------
AlmaLinux 8.4 [x86_64] (2019-2021)
----------------------------------

sudo yum install epel-release
sudo dnf config-manager --set-enabled PowerTools
sudo yum update

Package deps:
sudo yum install clang clang-analyzer valgrind txt2tags make automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qt5-qtsvg SDL2-devel cairo-devel pango-devel librsvg2-devel libpng-devel libjpeg-turbo-devel giflib-devel libsqlite3x-devel libsndfile-devel inkscape mesa-libGL-devel

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64" clean

* Smoke test the apps.

* Remove
	./build_fedora.sh remove clean


----------------------------------
openSUSE 15.3 XFCE [x86_64] (2021)
----------------------------------

Package deps:
sudo zypper install rpm-build automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools libqt5-qtbase-devel libSDL2-devel cairo-devel pango-devel librsvg2-devel libpng16-16 libpng-devel libjpeg62-devel libgif7 giflib-devel sqlite3-devel libsndfile-devel txt2tags inkscape Mesa-libGL-devel

* Install
	./build_suse.sh --conf "--libdir=/usr/lib64" clean

* Smoke test the apps.

* Remove
	./build_suse.sh remove clean


---------------------------------
Arco 20.11.9 [x86_64] (2020-2021)
---------------------------------

Package deps:
sudo pacman -S base-devel txt2tags qt5-base qt5-qtsvg qt6-base qt6-svg sdl2 clang-analyzer time valgrind cppcheck librsvg libpng giflib libjpeg sqlite libsndfile inkscape mesa

* Install
	./build_arch.sh clean

* Smoke test the apps.

* Remove
	./build_arch.sh remove clean


------------------------------------------------
Raspberry Pi OS (Debian 10) [ARM_32] (2019-2020)
------------------------------------------------

Package deps:
sudo apt-get install dh-make pbuilder clang clang-tools valgrind bison flex cppcheck txt2tags automake libreadline-dev qtbase5-dev libsdl2-dev time libpango1.0-dev librsvg2-dev libpng-dev libgif-dev libjpeg-dev libsqlite3-dev libsndfile1-dev inkscape mesa-common-dev libgl1-mesa-dev

* Install
	./build_install.sh --bcfile etc/bcfile_raspbian.txt

* Run ./test/* test suites to expose regressions.
	./configure
	make time
	./configure flush

* Smoke test the apps.

* Remove
	./build_install.sh --bcfile etc/bcfile_raspbian.txt remove
	./build_install.sh --bcfile etc/bcfile_raspbian.txt flush


--------------------------
Salix 14.2 [x86_32] (2016)
--------------------------

sudo slapt-get -i txt2tags inkscape qt5 SDL2

* Install, test, uninstall locally:

	export DIR="$HOME/test/usr"
	cd ..
	cd mtcedcli; cp configure.slackware12.0 configure; cd ..
	cd mtdwcli; cp configure.slackware12.0 configure; cd ..
	cd mtpixycli; cp configure.slackware12.0 configure; cd ..
	cd pkg

	./build_local.sh --preconf "LDFLAGS=-Wl,-rpath=$DIR/lib" --conf "--prefix=$DIR" clean

* Smoke test the apps.

* Remove
	./build_local.sh --preconf "LDFLAGS=-Wl,-rpath=$DIR/lib" --conf "--prefix=$DIR" clean remove
	rm -rf $DIR


-----
NOTES
---------------------
Slackware 12.0 - 14.2
---------------------
mtCedCLI doesn't build successfully due to an issue with the libreadline library.  For some reason the library has symbol references to libncurses, but with no explicit library link.  Therefore the linker generates an error when these symbols cannot be resolved.  This problem can be solved by editing the configure script.  I have put these changes into configure.slackware12.0 as a convenience.  This also means you cannot use '-Wl,--as-needed' because the linker doesn't really know what is going on.

