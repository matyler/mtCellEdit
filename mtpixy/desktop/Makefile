include		../_conf.txt

ICONDIR		= $(DESTDIR)$(DATA_INSTALL)/icons/hicolor
MTDESK		= $(BIN_NAME).desktop
MTDESKXML	= $(BIN_NAME).xml


.PHONY:		all install uninstall clean

all:		$(MTDESK) $(MTDESKXML)

$(MTDESK):	desktop.txt
		./builddesktop.sh "desktop.txt" "$(BIN_INSTALL)" "$(BIN_NAME)" "$(APP_NAME)" "$(BIN_SUFFIX)" > $(MTDESK)

$(MTDESKXML):	mime.xml.txt
		./builddesktop.sh "mime.xml.txt" "$(BIN_INSTALL)" "$(BIN_NAME)" "$(APP_NAME)" "$(BIN_SUFFIX)" > $(MTDESKXML)

install:
		install -d $(DESTDIR)$(DATA_INSTALL)/applications
		install -m 644 $(MTDESK) $(DESTDIR)$(DATA_INSTALL)/applications
		install -d $(DESTDIR)$(DATA_INSTALL)/mime/packages
		install -m 644 $(MTDESKXML) $(DESTDIR)$(DATA_INSTALL)/mime/packages
		install -d $(ICONDIR)/16x16/apps
		install -d $(ICONDIR)/32x32/apps
		install -d $(ICONDIR)/48x48/apps
		install -d $(ICONDIR)/64x64/apps
		install -d $(ICONDIR)/256x256/apps
		install -d $(ICONDIR)/scalable/apps
		install -m 644 16.png $(ICONDIR)/16x16/apps/$(BIN_NAME).png
		install -m 644 32.png $(ICONDIR)/32x32/apps/$(BIN_NAME).png
		install -m 644 48.png $(ICONDIR)/48x48/apps/$(BIN_NAME).png
		install -m 644 64.png $(ICONDIR)/64x64/apps/$(BIN_NAME).png
		install -m 644 256.png $(ICONDIR)/256x256/apps/$(BIN_NAME).png
		install -m 644 svg.svg $(ICONDIR)/scalable/apps/$(BIN_NAME).svg
		install -d $(ICONDIR)/16x16/mimetypes
		install -d $(ICONDIR)/32x32/mimetypes
		install -d $(ICONDIR)/48x48/mimetypes
		install -d $(ICONDIR)/64x64/mimetypes
		install -d $(ICONDIR)/256x256/mimetypes
		install -d $(ICONDIR)/scalable/mimetypes
		install -m 644 16.png $(ICONDIR)/16x16/mimetypes/application-x-$(BIN_NAME).png
		install -m 644 32.png $(ICONDIR)/32x32/mimetypes/application-x-$(BIN_NAME).png
		install -m 644 48.png $(ICONDIR)/48x48/mimetypes/application-x-$(BIN_NAME).png
		install -m 644 64.png $(ICONDIR)/64x64/mimetypes/application-x-$(BIN_NAME).png
		install -m 644 256.png $(ICONDIR)/256x256/mimetypes/application-x-$(BIN_NAME).png
		install -m 644 svg.svg $(ICONDIR)/scalable/mimetypes/application-x-$(BIN_NAME).svg

uninstall:
		rm $(DESTDIR)$(DATA_INSTALL)/applications/$(MTDESK)
		rm $(DESTDIR)$(DATA_INSTALL)/mime/packages/$(MTDESKXML)
		rm $(ICONDIR)/*/apps/$(BIN_NAME).png
		rm $(ICONDIR)/*/apps/$(BIN_NAME).svg
		rm $(ICONDIR)/*/mimetypes/application-x-$(BIN_NAME).png
		rm $(ICONDIR)/*/mimetypes/application-x-$(BIN_NAME).svg

clean:
		rm -rf *.desktop *.xml
