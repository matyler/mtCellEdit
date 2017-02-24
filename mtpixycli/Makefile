include		_conf.txt


.PHONY:		$(MAKE_DIRS) all install uninstall clean

all install uninstall clean:
		set -e; for DIR in $(MAKE_DIRS); do $(MAKE) -C $$DIR $@; done
