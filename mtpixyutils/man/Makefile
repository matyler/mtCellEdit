include		../_conf.txt

OBJ		=			\
		pixydelta.1		\
		pixyls.1		\
		pixynew.1		\
		pixypica.1		\
		pixyresize.1		\
		pixyriba.1		\
		pixyrida.1		\
		pixyrisa.1		\
		pixyscale.1		\
		pixytwit.1		\
		pixyutils.1		\


.PHONY:		all install uninstall clean


all:		$(OBJ)


.SUFFIXES:	.t2t .1


.t2t.1:
		./buildman.sh $< $(APP_VERSION) $(BIN_NAME) |		\
			txt2tags -t man -o - -i - > $@

install:
		install -d $(DESTDIR)$(MAN_INSTALL)
		install -m 644 $(OBJ) $(DESTDIR)$(MAN_INSTALL)
		cd $(DESTDIR)$(MAN_INSTALL); for FILE in $(OBJ);	\
			do gzip $$FILE; done

uninstall:
		cd $(DESTDIR)$(MAN_INSTALL); for FILE in $(OBJ);	\
			do rm $$FILE.gz; done

clean:

