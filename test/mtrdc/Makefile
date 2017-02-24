.PHONY:		all valg time install uninstall clean

CLEAN		= rm -f tmp/* valg_log.txt

all:
		$(CLEAN)
		./suite.sh

valg:
		$(CLEAN)
		VALG=Y ./suite.sh

time:
		VALG=T ./suite.sh

install:

uninstall:

clean:
		$(CLEAN)
