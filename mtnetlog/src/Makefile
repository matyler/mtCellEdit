include		../_conf.txt

CFLAGS		= $(BIN_CFLAGS) $(CFLAG)
LDFLAGS 	= $(LDFLAG)
OBJS		= main.o


.PHONY: 	all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CC) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

# .c.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f *.o $(BIN_NAME) *.tsv
