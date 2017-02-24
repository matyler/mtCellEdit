include		../_conf.txt


.PHONY:		all valg time install uninstall clean


# NOTE: - at the beginning of the instruction to ignore errors

all:
		-./suite.sh

valg:
		-VALG=Y ./suite.sh

time:
		-VALG=T ./suite.sh


# .c.o Implicit rule
# .cpp.o Implicit rule


install:

uninstall:

clean:
		rm -f output/* tmp/* valg_log.txt *.o *.tmp $(BIN_ALL)
