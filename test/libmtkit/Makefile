include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= mtstring
TMP_DIR		= tmp
TMP_FILES	= valg_log.txt log.txt *.o $(BIN_ALL)
PREPARE		= mkdir -p $(TMP_DIR)


all:		$(BIN_ALL)
		$(PREPARE)
		$(SUITE)

valg:		$(BIN_ALL)
		$(PREPARE)
		VALG=Y $(SUITE)

time:		$(BIN_ALL)
		$(PREPARE)
		VALG=T $(SUITE)

build:		$(BIN_ALL)

mtstring:	mtstring.o
		$(CC) mtstring.o -o mtstring $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

