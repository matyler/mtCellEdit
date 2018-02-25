include		../_conf.txt

OBJS		=			\
		cli_backend.o		\
		cli_jtf_clipboard.o	\
		cli_jtf_edit.o		\
		cli_jtf_file.o		\
		cli_jtf_misc.o		\
		cli_jtf_prefs.o		\
		cli_jtf_sheet.o		\
		cli_jtf_sort.o		\
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
		rm -f $(BIN_NAME) *.o
