This document outlines my testing procedures before this release.

* You cannot be root.  You must be a normal user with sudo rights.

* Your system must satisfy all of the requirements outlined in section 3 of the handbook.


--------------------------------
Debian 8 (amd64) [x86_64] (2015)
--------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libreadline6-dev libqt4-dev qtbase5-dev qtcreator qt4-qtconfig colorgcc time libpango1.0-dev libpng12-dev libgif-dev libjpeg-dev

* Install - old standards
	./build_debian.sh --preconf "LDFLAGS=-Wl,--as-needed" --conf "debugold --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4 --preconf "LDFLAGS=-Wl,--as-needed" --conf "debugold --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh flush

* Install - new standards
	./build_debian.sh --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4 --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh flush

* Test build using colourised warnings:
	./build_test.sh --preconf "CC=colorgcc CXX=colorgcc" --conf "debug"
	./build_install.sh flush

	./build_test.sh --preconf "CC=colorgcc CXX=colorgcc" --conf "debug --use-qt4" libmtqex4 mtcelledit-qt4 mteleana-qt4 mtraft-qt4 mtpixy-qt4
	./build_install.sh flush

* Build using clang-static to expose possible errors:
	./clang_scan.sh
	firefox /tmp/scan-build-*/index.html
	./build_install.sh flush

* Use cppcheck to expose programmer errors and unused functions:
	./cppcheck.sh > ~/cppcheck.txt 2>&1
	less ~/cppcheck.txt
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
	./build_debian.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4
	./build_debian.sh remove
	./build_debian.sh flush


-----------------------------------
Manjaro Linux 16.06 [x86_64] (2016)
-----------------------------------

Package deps: base-devel txt2tags qt5-base qt4 clang-analyzer colorgcc time valgrind cppcheck libpng giflib libjpeg

* Install
	./build_arch.sh
	./build_arch.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4
	./build_arch.sh flush

* Smoke test the apps.

* Remove
	./build_arch.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4
	./build_arch.sh remove
	./build_arch.sh flush


-------------------------------
CentOS 7.2 XFCE [x86_64] (2015)
-------------------------------

Package deps: clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck readline-devel rpmdevtools qt5-qtbase-devel qtchooser qt-devel qt-config cairo-devel pango-devel libpng-devel libjpeg-turbo-devel giflib-devel

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64"
	./build_fedora.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4 --conf "--libdir=/usr/lib64"

* Smoke test the apps.

* Remove
	./build_fedora.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 mtpixy-qt4
	./build_fedora.sh remove
	./build_fedora.sh flush


-----------------------------------------
Debian 7 (i386 256MB RAM) [x86_32] (2013)
-----------------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libreadline6-dev libqt4-dev qtcreator qt4-qtconfig colorgcc time libpango1.0-dev libpng12-dev libgif-dev libjpeg-dev

* Install
	./build_debian.sh --bcfile bcfile_debian7_i386.txt
	./build_debian.sh flush

* Run ./test/* test suites to expose regressions.
	./configure
	make time
	./configure flush

* Smoke test the apps.

* Remove
	./build_debian.sh --bcfile bcfile_debian7_i386.txt remove
	./build_debian.sh --bcfile bcfile_debian7_i386.txt flush


------------------------------------------------------
Debian 7 (armhf vexpress-a9 256MB RAM) [ARM_32] (2013)
------------------------------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libreadline6-dev libqt4-dev qtcreator qt4-qtconfig colorgcc time libpango1.0-dev libpng12-dev libgif-dev libjpeg-dev

* Install
	./build_install.sh --bcfile bcfile_debian7_arm.txt

* Run ./test/* test suites to expose regressions.
	./configure
	make time
	./configure flush

* Smoke test the apps.

* Remove
	./build_install.sh --bcfile bcfile_debian7_arm.txt remove
	./build_install.sh --bcfile bcfile_debian7_arm.txt flush


--------------------------------------------
Debian 7 (powerpc 256MB RAM) [PPC_32] (2013)
--------------------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libreadline6-dev libqt4-dev qtcreator qt4-qtconfig colorgcc time libpango1.0-dev libpng12-dev libgif-dev libjpeg-dev

* Install
	./build_install.sh --bcfile bcfile_debian7_ppc.txt

* Run ./test/* test suites to expose regressions.
	./configure
	make time
	./configure flush

* Smoke test the apps.

* Remove
	./build_install.sh --bcfile bcfile_debian7_ppc.txt remove
	./build_install.sh --bcfile bcfile_debian7_ppc.txt flush


-----
NOTES
---------------------
Slackware 12.0 - 14.1
---------------------
mtCedCLI doesn't build successfully due to an issue with the libreadline library.  For some reason the library has symbol references to libncurses, but with no explicit library link.  Therefore the linker generates an error when these symbols cannot be resolved.  This problem can be solved by editing the configure script.  I have put these changes into configure.slackware12.0 as a convenience.  This also means you cannot use '-Wl,--as-needed' because the linker doesn't really know what is going on.

----------------------------
Debian 7 (armhf vexpress-a9)
----------------------------
Clang seems to build bad code which doesn't run properly.  Valgrind is very unreliable.

----------
References
----------
https://wiki.debian.org/Hardening#User_Space
https://wiki.ubuntu.com/Security/Features
http://www.akkadia.org/drepper/dsohowto.pdf - "How To Write Shared Libraries" by Ulrich Drepper
http://www.akkadia.org/drepper/goodpractice.pdf
http://www.akkadia.org/drepper/defprogramming.pdf

