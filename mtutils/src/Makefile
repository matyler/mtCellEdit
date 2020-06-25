include		../_conf.txt

OBJS_GENTEX	:= $(patsubst %.c,%.o,$(wildcard mtgentex*.c))
OBJS_PTS2PLY	:= $(patsubst %.cpp,%.o,$(wildcard mtpts2ply*.cpp))
BINS		=			\
		mtbinana		\
		mtchls			\
		mtdsize			\
		mtgentex		\
		mtpts2ply		\

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


mtpts2ply:	$(OBJS_PTS2PLY)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_PTS2PLY): mtpts2ply.h


# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BINS) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL)/; rm $(BINS)

clean:
		rm -f $(BINS) *.o
