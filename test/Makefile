include		_conf.txt


.PHONY:		$(MAKE_DIRS) all valg time clean


all valg time:
		set -e; for DIR in $(MAKE_DIRS); do $(MAKE) -C $$DIR $@; done
		./results.sh > log.txt
		cat log.txt */log.txt | less -r

clean:
		set -e; for DIR in $(MAKE_DIRS); do $(MAKE) -C $$DIR $@; done
		rm -f log.txt
