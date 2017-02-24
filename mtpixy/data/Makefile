include ../_conf.txt


.PHONY:		all install uninstall clean

all:
		cp -r data $(DATA_NAME)
		chmod -R a+r $(DATA_NAME)
		find $(DATA_NAME) -type d -exec chmod a+x {} \;

install:
		install -d $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)
		cp -r $(DATA_NAME)/* $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)

uninstall:
		rm -r $(DESTDIR)$(DATA_INSTALL)/$(DATA_NAME)

clean:
		rm -rf $(DATA_NAME)
