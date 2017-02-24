include		../_conf.txt

OBJS		=			\
		cli_backend.o		\
		cli_jtf.o		\
		cli_jtf_image.o		\
		cli_jtf_paint.o		\
		cli_jtf_palette.o	\
		cli_jtf_selection.o	\
		cli_main.o		\


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CXX) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

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
