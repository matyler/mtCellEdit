include		../_conf.txt

OBJ		:= $(sort $(patsubst %.t2t,%.1,$(wildcard *.t2t)))

.PHONY:		all install uninstall clean


all:		$(OBJ)


%.1:		%.t2t
		./buildman.sh $< $(APP_VERSION)		|		\
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
