include		../_conf.txt


.PHONY: 	all valg time build install uninstall clean


CFLAGS		= $(BIN_CFLAGS) $(CFLAG)
LDFLAGS 	= $(LDFLAG)
BIN_ALL		= mtgentex


# NOTE: - at the beginning of the instruction to ignore errors

all:		$(BIN_ALL)
		-./suite.sh

valg:		$(BIN_ALL)
		-VALG=Y ./suite.sh

time:		$(BIN_ALL)
		-VALG=T ./suite.sh

build:		$(BIN_ALL)

mtgentex:	mtgentex_main.o mtgentex_func.o
		$(CC) mtgentex_main.o mtgentex_func.o -o mtgentex $(LDFLAGS)

# .c.o Implicit rule

install:

uninstall:

clean:
		rm -f output/* tmp/* valg_log.txt *.o *.tmp $(BIN_ALL)
