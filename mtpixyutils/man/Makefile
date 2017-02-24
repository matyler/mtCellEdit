include		../_conf.txt

OBJ		=			\
		pixyls.1.gz		\
		pixynew.1.gz		\
		pixyresize.1.gz		\
		pixyscale.1.gz		\
		pixyutils.1.gz		\


.PHONY:		all install uninstall clean


all:		$(OBJ)


.SUFFIXES:	.t2t .1.gz


.t2t.1.gz:
		./buildman.sh $< $(APP_VERSION) $(BIN_NAME) | txt2tags -t man -o - -i - | gzip > $@

install:
		install -d $(DESTDIR)$(MAN_INSTALL)
		install -m 644 $(OBJ) $(DESTDIR)$(MAN_INSTALL)

uninstall:
		rm $(DESTDIR)$(MAN_INSTALL)/$(OBJ)

clean:

