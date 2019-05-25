include		../_conf.txt

OBJS_GENTEX	:= $(patsubst %.c,%.o,$(wildcard mtgentex*.c))
BINS		=			\
		mtbinana		\
		mtchls			\
		mtdsize			\
		mtgentex		\

.PHONY:		all install uninstall clean


all:		$(BINS)


mtbinana:	mtbinana.o
		$(CXX) $^ -o $@ $(LDFLAGS)

mtchls:		mtchls.o
		$(CXX) $^ -o $@ $(LDFLAGS)

mtchls.o:	mtchls.h

%:		%.py
		./buildpy.sh $< $(APP_VERSION) > $@
		chmod a+x $@

mtgentex:	$(OBJS_GENTEX)
		$(CC) $^ -o $@ $(LDFLAGS)

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
