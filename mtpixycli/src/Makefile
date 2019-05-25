include		../_conf.txt

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

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f *.o $(BIN_NAME)
