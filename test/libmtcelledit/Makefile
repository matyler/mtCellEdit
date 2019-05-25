include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= tcreadst tcreadmt tcwritest tcwritemt
TMP_DIR		= tmp
TMP_FILES	= valg_log.txt log.txt $(BIN_ALL) *.o *.tsv
PREPARE		= mkdir -p $(TMP_DIR)

TCREADST_OBJ	=			\
		thrd_build.o		\
		thrd_tcread_st.o	\

TCREADMT_OBJ	=			\
		thrd_build.o		\
		thrd_tcread_mt.o	\

TCWRITEST_OBJ	=			\
		thrd_build.o		\
		thrd_tcwrite_st.o	\

TCWRITEMT_OBJ	=			\
		thrd_build.o		\
		thrd_tcwrite_mt.o	\


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

$(TCREADST_OBJ):	*.h

$(TCREADMT_OBJ):	*.h

$(TCWRITEST_OBJ):	*.h

$(TCWRITEMT_OBJ):	*.h

tcreadst:	$(TCREADST_OBJ)
		$(CC) $(TCREADST_OBJ) -o $@ $(LDFLAGS)

tcreadmt:	$(TCREADMT_OBJ)
		$(CC) $(TCREADMT_OBJ) -o $@ $(LDFLAGS)

tcwritest:	$(TCWRITEST_OBJ)
		$(CC) $(TCWRITEST_OBJ) -o $@ $(LDFLAGS)

tcwritemt:	$(TCWRITEMT_OBJ)
		$(CC) $(TCWRITEMT_OBJ) -o $@ $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

