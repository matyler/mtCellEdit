include		../_conf.txt

OBJS		=		\
		cli_clipboard.o	\
		cli_edit.o	\
		cli_file.o	\
		cli_main.o	\
		cli_misc.o	\
		cli_parse.o	\
		cli_prefs.o	\
		cli_sheet.o	\
		cli_sort.o	\
		cli_tab.o	\


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
