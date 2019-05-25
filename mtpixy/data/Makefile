include ../_conf.txt

MAKE_DIRS	= data


.PHONY:		all install uninstall clean

all:		clean
		cp -r data $(DATA_NAME)
		chmod -R a+r $(DATA_NAME)
		find $(DATA_NAME) -type d -exec chmod a+x {} \;
		set -e; for dir in $(DATA_NAME); do $(MAKE) -C $$dir $@; done
		rm $(DATA_NAME)/Makefile
		rm $(DATA_NAME)/*/Makefile
		rm $(DATA_NAME)/*/build.sh

install:
		install -d $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)
		cp -r $(DATA_NAME)/* $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)

uninstall:
		rm -r $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)

clean:
		rm -rf $(DATA_NAME)
		set -e; for dir in $(MAKE_DIRS); do $(MAKE) -C $$dir $@; done

