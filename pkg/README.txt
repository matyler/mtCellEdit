This document outlines my testing procedures before this release.

* You cannot be root.  You must be a normal user with sudo rights.

* Your system must satisfy all of the requirements outlined in section 3 of the handbook.


Test Systems:
	DEB
		Debian 12 (amd64) [x86_64] (2023-2024)
	RPM
		CentOS 7.9 XFCE [x86_64] (2014-2022)		+ AppImage
		Fedora 40 XFCE [x86_64] (2024)
		openSUSE Tumbleweed XFCE [x86_64] (2024-Jan)
	Arch
		Manjaro 23.1 [x86_64] (2023-2024)
	Local install
		Salix 15.0 [x86_64] (2022-2024)
	bcfile install
		Raspberry Pi OS (Debian 12) [ARM_32] (2023-2024)


---------------------------------
Debian 12 (amd64) [x86_64] (2023)
---------------------------------

Package deps:
sudo apt-get install dh-make pbuilder clang clang-tools valgrind bison flex cppcheck txt2tags automake libreadline-dev qtbase5-dev qt6-base-dev libsdl2-dev qtcreator time libpango1.0-dev librsvg2-dev libpng-dev libgif-dev libjpeg-dev libsqlite3-dev libsndfile1-dev inkscape mesa-common-dev libgl1-mesa-dev libgmp-dev libmpfr-dev gperf

* Install
	./build_debian.sh --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu" clean all

* Build using clang-static to expose possible errors:
	./clang_scan.sh
	firefox /tmp/scan-build-*/index.html
	./build_install.sh flush

* Use cppcheck to expose programmer errors and unused functions:
	./cppcheck.sh > ~/cppcheck.txt 2>&1
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
Fedora 40 XFCE [x86_64] (2024)
------------------------------

Package deps:
sudo yum install clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qt5-qtsvg qt6-qtbase-devel qt6-qtsvg SDL2-devel cairo-devel pango-devel librsvg2-devel libpng-devel libjpeg-turbo-devel giflib-devel libsqlite3x-devel libsndfile-devel inkscape mesa-libGL-devel libubsan gmp-devel mpfr-devel gperf

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64 debug" clean all

* Smoke test the apps.

* Remove
	./build_fedora.sh remove clean all


------------------------------------
CentOS 7.9 XFCE [x86_64] (2014-2022)
------------------------------------

Package deps:
sudo yum install clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qt5-qtsvg SDL2-devel cairo-devel pango-devel librsvg2-devel libpng-devel libjpeg-turbo-devel giflib-devel libsqlite3x-devel libsndfile-devel inkscape mesa-libGL-devel gmp-devel mpfr-devel gperf

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64" clean

* Smoke test the apps.

* Remove
	./build_fedora.sh remove clean

* Build AppImages
	./build_appimage_all.sh
	./build_install.sh flush


--------------------------------------------
openSUSE Tumbleweed XFCE [x86_64] (2024-Jan)
--------------------------------------------

Package deps:
sudo zypper install rpm-build automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools libqt5-qtbase-devel qt6-base-devel libQt6Svg6 libSDL2-devel cairo-devel pango-devel librsvg2-devel libpng16-16 libpng-devel libjpeg62-devel libgif7 giflib-devel sqlite3-devel libsndfile-devel txt2tags inkscape Mesa-libGL-devel gmp-devel mpfr-devel gperf

* Install
	./build_suse.sh --conf "--libdir=/usr/lib64" clean all

* Smoke test the apps.

* Remove
	./build_suse.sh remove clean all


---------------------------------
Manjaro 23.1 [x86_64] (2023-2024)
---------------------------------

Package deps:
sudo pacman -S base-devel txt2tags qt5-base qt5-svg qt6-base qt6-svg sdl2 clang-analyzer time valgrind cppcheck librsvg libpng giflib libjpeg sqlite libsndfile inkscape mesa gmp mpfr gperf

* Install
	./build_arch.sh all clean

* Smoke test the apps.

* Remove
	./build_arch.sh remove all clean


------------------------------------------------
Raspberry Pi OS (Debian 12) [ARM_32] (2023-2024)
------------------------------------------------

Package deps:
sudo apt-get install dh-make pbuilder clang clang-tools valgrind bison flex cppcheck txt2tags automake libreadline-dev qtbase5-dev qt6-base-dev libsdl2-dev qtcreator time libpango1.0-dev librsvg2-dev libpng-dev libgif-dev libjpeg-dev libsqlite3-dev libsndfile1-dev inkscape mesa-common-dev libgl1-mesa-dev libgmp-dev libmpfr-dev gperf

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


-------------------------------
Salix 15.0 [x86_64] (2022-2024)
-------------------------------

sudo slapt-get -i txt2tags inkscape qt5 SDL2

* Install, test, uninstall locally:

	DIR="$HOME/test/usr"
	./build_local.sh --preconf "LDFLAGS=-Wl,-rpath=$DIR/lib" --conf "--prefix=$DIR" clean

* Smoke test the apps.

* Remove
	./build_local.sh --preconf "LDFLAGS=-Wl,-rpath=$DIR/lib" --conf "--prefix=$DIR" clean remove
	rm -rf $DIR


