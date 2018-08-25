include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= mkdata

MKDATA_OBJ	=			\
		mkdata.o		\
		mkdata_flac.o		\
		mkdata_png.o		\

all:		$(BIN_ALL)
		$(SUITE)

valg:		$(BIN_ALL)
		VALG=Y $(SUITE)

time:		$(BIN_ALL)
		VALG=T $(SUITE)

build:		$(BIN_ALL)

$(MKDATA_OBJ):	*.h

mkdata:		$(MKDATA_OBJ)
		$(CXX) $(MKDATA_OBJ) -o $@ $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -rf *.o valg*.txt log.txt d0/* d1/* d2/* d3/*	\
			output/* tmp/* $(BIN_ALL)
