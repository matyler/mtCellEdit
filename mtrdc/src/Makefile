include		../_conf.txt

CFLAGS		= $(BIN_CFLAGS) $(CFLAG)
LDFLAGS 	= $(LDFLAG)
OBJS		=		\
		rdc_analysis.o	\
		rdc_main.o	\
		rdc_matrix.o	\
		rdc_password.o	\
		rdc_pad.o	\
		rdc_prng.o	\
		rdc_shuffle.o	\
		rdc_xor.o	\


.PHONY: 	all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CC) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

$(OBJS):	*.h

# .c.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f *.o $(BIN_NAME)
