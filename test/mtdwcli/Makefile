include		../_conf.txt


.PHONY:		all valg time build install uninstall clean


BIN_ALL		= mkdata rndfile
TMP_DIR		= d0 d1 d2 d3 output tmp r1 r2 r3 h1 h2 h3
TMP_FILES	= valg_log.txt log.txt $(BIN_ALL) *.o
PREPARE		= mkdir -p $(TMP_DIR)
MKDATA_OBJ	:= $(sort $(patsubst %.cpp,%.o,$(wildcard mkdata*.cpp)))
RNDFILE_OBJ	:= $(sort $(patsubst %.cpp,%.o,$(wildcard rndfile*.cpp)))


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

$(MKDATA_OBJ):	mkdata.h

mkdata:		$(MKDATA_OBJ)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(RNDFILE_OBJ):	rndfile.h

rndfile:	$(RNDFILE_OBJ)
		$(CXX) $^ -o $@ $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

