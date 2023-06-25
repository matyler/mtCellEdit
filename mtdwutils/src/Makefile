include		../_conf.txt

APPS		:=		\
		dwdecbot	\
		dwdecfont	\
		dwdechg		\
		dwdecsoda	\
		dwencbot	\
		dwencfont	\
		dwenchg		\
		dwencsoda	\

OBJS		:= $(sort $(patsubst %.cpp,%.o,$(wildcard dwutils*.cpp)))
OBJS_CLI	:= $(patsubst %.cpp,%.o,$(wildcard cli*.cpp))
OBJS_NUSHUTILS	:= $(patsubst %.cpp,%.o,$(wildcard nush*.cpp))

BINS		:= $(BIN_NAME) nushutils mtdwcli


.PHONY:		all install uninstall clean


all:		$(BINS)


$(BIN_NAME):	$(OBJS)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS):	dwu*.h

nushutils:	$(OBJS_NUSHUTILS)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_NUSHUTILS): nush*.h

mtdwcli:	$(OBJS_CLI)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_CLI):	cli*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BINS) $(DESTDIR)$(BIN_INSTALL)
		cd $(DESTDIR)$(BIN_INSTALL); for APP in $(APPS); do ln -sfv $(BIN_NAME) $$APP; done

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL); rm $(BINS) $(APPS)

clean:
		rm -f *.o $(BINS)
