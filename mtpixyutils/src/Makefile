include		../_conf.txt

APPS		:=		\
		pixycmp		\
		pixydelta	\
		pixyfade	\
		pixyls		\
		pixynew		\
		pixypica	\
		pixyresize	\
		pixyriba	\
		pixyrida	\
		pixyrisa	\
		pixyscale	\

OBJS		:= $(sort $(patsubst %.cpp,%.o,$(wildcard pixyutils*.cpp)))
OBJS_CLI	:= $(sort $(patsubst %.cpp,%.o,$(wildcard cli*.cpp)))

BINS		:= $(BIN_NAME) mtpixycli


.PHONY:		all install uninstall clean


all:		$(BINS)


$(BIN_NAME):	$(OBJS)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS):	pixyu*.h

mtpixycli:	$(OBJS_CLI)
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
