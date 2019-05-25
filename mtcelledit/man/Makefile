include		../_conf.txt

SRC		:= *.t2t
OBJ		:= $(BIN_NAME).1

.PHONY:		all install uninstall clean


all:		$(OBJ)


$(OBJ):		$(SRC)
		./buildman.sh $< $(APP_VERSION) $(BIN_NAME) |		\
			txt2tags -t man -o - -i - > $@

install:
		install -d $(DESTDIR)$(MAN_INSTALL)
		install -m 644 $(OBJ) $(DESTDIR)$(MAN_INSTALL)
		cd $(DESTDIR)$(MAN_INSTALL); for FILE in $(OBJ);	\
			do gzip -f $$FILE; done

uninstall:
		cd $(DESTDIR)$(MAN_INSTALL); for FILE in $(OBJ);	\
			do rm $$FILE.gz; done

clean:
		rm -f *.1
