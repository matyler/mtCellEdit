include		../_conf.txt

OBJ		=			\
		cedappend.1		\
		cedclear.1		\
		cedcut.1		\
		ceddiff.1		\
		cedeval.1		\
		cedfind.1		\
		cedflip.1		\
		cedfuzzmap.1		\
		cedinsert.1		\
		cedls.1			\
		cedpaste.1		\
		cedrotate.1		\
		cedset.1		\
		cedsort.1		\
		cedtranspose.1		\
		cedutils.1		\
		mtnetlog.1		\
		mtrdc.1			\


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

