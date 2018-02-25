include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= tcreadst tcreadmt tcwritest tcwritemt

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
		$(SUITE)

valg:		$(BIN_ALL)
		VALG=Y $(SUITE)

time:		$(BIN_ALL)
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
		rm -f *.o *.tsv valg*.txt log.txt tmp/* $(BIN_ALL)
