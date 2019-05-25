include		../_conf.txt

INSTALL_DIR	= $(DESTDIR)$(DATA_INSTALL)/doc/$(BIN_NAME)/
MAKE_DIRS	= en_GB


.PHONY:		$(MAKE_DIRS) all install uninstall clean


all:
		mkdir -p img
		cp ../desktop/svg.svg img
		set -e; for dir in $(MAKE_DIRS); do $(MAKE) -C $$dir $@; done

install:
		mkdir -p $(INSTALL_DIR)
		cp -r img $(INSTALL_DIR)
		set -e; for dir in $(MAKE_DIRS); do mkdir -p $(INSTALL_DIR)/$$dir; cp $$dir/*.html $(INSTALL_DIR)/$$dir; done
		chmod -R a+r $(INSTALL_DIR)
		find $(INSTALL_DIR) -type d -exec chmod a+x {} \;

uninstall:
		rm -rf $(INSTALL_DIR)

clean:
		rm -rf img
		set -e; for dir in $(MAKE_DIRS); do $(MAKE) -C $$dir $@; done
