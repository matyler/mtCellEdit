include		../_conf.txt


.PHONY:		all valg time install uninstall clean


CLEAN		= rm -f tmp/* valg_log.txt log.txt


all:
		$(CLEAN)
		$(SUITE)

valg:
		$(CLEAN)
		VALG=Y $(SUITE)

time:
		VALG=T $(SUITE)

install:

uninstall:

clean:
		$(CLEAN)
