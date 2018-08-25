

.PHONY:		all install uninstall clean man handbook root core

all:		man handbook root core

core:
		mkdir -p out tmp

man:		core
		./build_man.sh

handbook:	core
		./build_handbook.sh

root:		man handbook
		./build_root.sh

install:

uninstall:

clean:
		rm -rf out tmp

