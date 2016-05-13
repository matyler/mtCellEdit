This document outlines my testing procedures before this release.

* You cannot be root.  You must be a normal user with sudo rights.

* Your system must satisfy all of the requirements outlined in section 3 of the handbook.


-----------------------------------
Manjaro Linux 15.12 [x86_64] (2015)
-----------------------------------

Package deps: base-devel txt2tags qt5-base qt4 clang-analyzer colorgcc time valgrind cppcheck

* Install
	./build_arch.sh --conf "debug"
	./build_arch.sh --conf "debug" libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft
	./build_arch.sh flush

* Test build using colourised warnings:
	./build_test.sh --preconf "CC=colorgcc CXX=colorgcc" --conf "debug"
	./build_install.sh flush

	./build_test.sh --preconf "CC=colorgcc CXX=colorgcc" --conf "debug --enable-qt4" libmtqex4 mtcelledit-qt4 mteleana-qt4 mtraft-qt4
	./build_install.sh flush

* Build using clang-static to expose possible errors:

	./clang_scan.sh
	firefox /tmp/scan-build-*/index.html
	./build_install.sh flush

* Use cppcheck to expose programmer errors:
	./cppcheck.sh > ~/cppcheck.txt 2>&1
	less ~/cppcheck.txt
	rm ~/cppcheck.txt

* Use readelf to study all libs and bins in search of cruft or overexposed private functions:
	./readelf_cmp.sh --libdir /usr/lib
	./readelf_cmp.sh flush

* Run ./test/* test suites to expose regressions.
	./configure debug
	make valg
	make clean
	make time
	./configure flush

* Smoke test the apps.

* Skim documentation to expose cruft and mistakes.

* Remove
	./build_arch.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft
	./build_arch.sh remove
	./build_arch.sh flush


-------------------------------------
Lubuntu 16.04 (amd64) [x86_64] (2016)
-------------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libgtk-3-dev libgtk2.0-dev libreadline6-dev libqt4-dev qtbase5-dev qtcreator qt4-qtconfig colorgcc time

* Install
	./build_debian.sh --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft --preconf "LDFLAGS=-Wl,--as-needed" --conf "debug --libdir=/usr/lib/x86_64-linux-gnu"
	./build_debian.sh flush

* Run ./test/* test suites to expose regressions.
	./configure debug
	make valg
	make clean
	make time
	./configure flush

* Remove
	./build_debian.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft
	./build_debian.sh remove
	./build_debian.sh flush


------------------------------
Fedora 23 XFCE [x86_64] (2015)
------------------------------

Package deps: clang clang-analyzer valgrind txt2tags automake gcc gcc-c++ bison flex cppcheck gtk2-devel readline-devel rpmdevtools qt5-qtbase-devel qtchooser qt-devel qt-config

* Install
	./build_fedora.sh --conf "--libdir=/usr/lib64"
	./build_fedora.sh libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft --conf "--libdir=/usr/lib64"

* Smoke test the apps.

* Remove
	./build_fedora.sh remove libmtqex4 mtcelledit-qt4 mtraft-qt4 mteleana-qt4 libmtgex mtcelledit mtraft
	./build_fedora.sh remove
	./build_fedora.sh flush


------------------------------------------------------
Debian 7 (armhf vexpress-a9 256MB RAM) [ARM_32] (2013)
------------------------------------------------------

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libgtk-3-dev libgtk2.0-dev libreadline6-dev libqt4-dev qtcreator qt4-qtconfig colorgcc time

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

Package deps: dh-make pbuilder clang valgrind bison flex cppcheck txt2tags automake libgtk-3-dev libgtk2.0-dev libreadline6-dev libqt4-dev qtcreator qt4-qtconfig colorgcc time

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


---------------------------
PC-BSD 10.0 [x86_64] (2014)
---------------------------

Package deps: devel/qt4

* Install
	./build_install.sh --bcfile bcfile_freebsd.txt

* Smoke test the apps.

* Remove
	./build_install.sh --bcfile bcfile_freebsd.txt remove
	./build_install.sh --bcfile bcfile_freebsd.txt flush


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

