include		../_conf.txt

SRC		= mansrc.t2t
OBJ		= $(BIN_NAME).1.gz


.PHONY:		all install uninstall clean


all:		$(OBJ)


$(OBJ):		$(SRC)
		./buildman.sh $< $(APP_VERSION) $(BIN_NAME) | txt2tags -t man -o - -i - | gzip > $@

install:
		install -d $(DESTDIR)$(MAN_INSTALL)
		install -m 644 $(OBJ) $(DESTDIR)$(MAN_INSTALL)

uninstall:
		rm $(DESTDIR)$(MAN_INSTALL)/$(OBJ)

clean:
