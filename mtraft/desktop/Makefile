include		../_conf.txt


ICONDIR		= $(DESTDIR)$(DATA_INSTALL)/icons/hicolor
MTDESK		= $(BIN_NAME).desktop


.PHONY:		all install uninstall clean

all:		$(MTDESK)

$(MTDESK):	mtraft.desktop.txt
		./builddesktop.sh "mtraft.desktop.txt" "$(BIN_INSTALL)" "$(BIN_NAME)" "$(APP_NAME)" "$(BIN_SUFFIX)" > $(MTDESK)

install:
		install -d $(DESTDIR)$(DATA_INSTALL)/applications
		install -m 644 $(MTDESK) $(DESTDIR)$(DATA_INSTALL)/applications
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

uninstall:
		rm $(DESTDIR)$(DATA_INSTALL)/applications/$(MTDESK)
		rm $(ICONDIR)/*/apps/$(BIN_NAME).png
		rm $(ICONDIR)/*/apps/$(BIN_NAME).svg

clean:
		rm -rf *.desktop
