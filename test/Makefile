include		_conf.txt


.PHONY:		$(MAKE_DIRS) all valg time clean

all valg time clean:
		set -e; for DIR in $(MAKE_DIRS); do $(MAKE) -C $$DIR $@; done
