include		../_conf.txt


OBJS_GENTEX	=			\
		mtgentex_func.o		\
		mtgentex_main.o		\

BINS		=			\
		mtbinana		\
		mtchls			\
		mtdsize			\
		mtgentex		\


.PHONY:		all install uninstall clean

all:		$(BINS)

mtbinana:	mtbinana.o
		$(CXX) mtbinana.o -o $@ $(LDFLAGS)

mtchls:		mtchls.o
		$(CXX) mtchls.o -o $@ $(LDFLAGS)

mtchls.o:	mtchls.h

mtdsize:	mtdsize.py
		./buildpy.sh mtdsize.py $(APP_VERSION) > $@
		chmod a+x $@

mtgentex:	$(OBJS_GENTEX)
		$(CC) $(OBJS_GENTEX) -o $@ $(LDFLAGS)

$(OBJS_GENTEX):	mtgentex.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BINS) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL)/; rm $(BINS)

clean:
		rm -f $(BINS) *.o

