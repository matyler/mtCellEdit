include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= mtstring

all:		$(BIN_ALL)
		$(SUITE)

valg:		$(BIN_ALL)
		VALG=Y $(SUITE)

time:		$(BIN_ALL)
		VALG=T $(SUITE)

build:		$(BIN_ALL)

mtstring:	mtstring.o
		$(CC) mtstring.o -o mtstring $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -f *.o *.txt tmp/* $(BIN_ALL)
