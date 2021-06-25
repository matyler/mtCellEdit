include		../_conf.txt

OBJS_LANTER	:= $(patsubst %.cpp,%.o,$(wildcard lanter*.cpp))
OBJS_MANDY	:= $(patsubst %.cpp,%.o,$(wildcard mandy*.cpp))
OBJS_SOREC	:= $(patsubst %.cpp,%.o,$(wildcard sorec*.cpp))
OBJS_SOUP	:= $(patsubst %.cpp,%.o,$(wildcard soup*.cpp))

BINS		= mtsorec mtsoup mtmandy mtlanter

.PHONY:		all install uninstall clean


all:		$(BINS)


mtlanter:	$(OBJS_LANTER)
		$(CXX) $^ -o $@ $(LDFLAGS)

mtmandy:	$(OBJS_MANDY)
		$(CXX) $^ -o $@ $(LDFLAGS)

mtsorec:	$(OBJS_SOREC)
		$(CXX) $^ -o $@ $(LDFLAGS)

mtsoup:		$(OBJS_SOUP)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_LANTER):	lanter.h
$(OBJS_MANDY):	mandy.h
$(OBJS_SOREC):	sorec.h
$(OBJS_SOUP):	soup.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BINS) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL)/; rm $(BINS)

clean:
		rm -f *.o $(BINS)
