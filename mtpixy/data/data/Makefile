MAKE_DIRS	= icons


.PHONY:		all install uninstall clean


all install uninstall clean:
		set -e; for dir in $(MAKE_DIRS); do $(MAKE) -C $$dir $@; done

