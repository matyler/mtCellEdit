include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= mtstring


all:		$(BIN_ALL)
		./suite.sh

valg:		$(BIN_ALL)
		VALG=Y ./suite.sh

time:		$(BIN_ALL)
		VALG=T ./suite.sh

build:		$(BIN_ALL)

mtstring:	mtstring.o
		$(CXX) mtstring.o -o mtstring $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -f *.o *.txt tmp/* $(BIN_ALL)
