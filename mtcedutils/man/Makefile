include		../_conf.txt

OBJ		=			\
		cedappend.1.gz		\
		cedclear.1.gz		\
		cedcut.1.gz		\
		ceddiff.1.gz		\
		cedeval.1.gz		\
		cedfind.1.gz		\
		cedflip.1.gz		\
		cedinsert.1.gz		\
		cedls.1.gz		\
		cedpaste.1.gz		\
		cedrotate.1.gz		\
		cedset.1.gz		\
		cedsort.1.gz		\
		cedtranspose.1.gz	\
		cedutils.1.gz		\


.PHONY:		all install uninstall clean


all:		$(OBJ)


.SUFFIXES:	.t2t .1.gz


.t2t.1.gz:
		./buildman.sh $< $(APP_VERSION) $(BIN_NAME) | txt2tags -t man -o - -i - | gzip > $@

install:
		install -d $(DESTDIR)$(MAN_INSTALL)
		install -m 644 $(OBJ) $(DESTDIR)$(MAN_INSTALL)

uninstall:
		cd $(DESTDIR)$(MAN_INSTALL); rm $(OBJ)

clean:

