include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= numbin numtest
TMP_DIR		= output tmp
TMP_FILES	= valg_log.txt log.txt $(BIN_ALL) *.o
PREPARE		= mkdir -p $(TMP_DIR)
NUMBIN_OBJ	:= $(sort $(patsubst %.cpp,%.o,$(wildcard numbin*.cpp)))
NUMTEST_OBJ	:= $(sort $(patsubst %.cpp,%.o,$(wildcard numtest*.cpp)))


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

$(NUMBIN_OBJ):	numbin.h

numbin:		$(NUMBIN_OBJ)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(NUMTEST_OBJ):	numtest.h

numtest:	$(NUMTEST_OBJ)
		$(CXX) $^ -o $@ $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

