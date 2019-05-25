include		../_conf.txt

APPS		:=		\
		pixydelta	\
		pixyfade	\
		pixyls		\
		pixynew		\
		pixypica	\
		pixyresize	\
		pixyriba	\
		pixyrisa	\
		pixyscale	\
		pixytwit	\

OBJS		:= $(sort $(patsubst %.cpp,%.o,$(wildcard *.cpp)))

.PHONY:		all install uninstall clean


all:		$(BIN_NAME)


$(BIN_NAME):	$(OBJS)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)
		cd $(DESTDIR)$(BIN_INSTALL); for APP in $(APPS); do ln -sfv $(BIN_NAME) $$APP; done

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL); rm $(BIN_NAME) $(APPS)

clean:
		rm -f *.o $(BIN_NAME)
