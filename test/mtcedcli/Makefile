include		../_conf.txt


.PHONY:		all valg time install uninstall clean


# NOTE: - at the beginning of the instruction to ignore errors

all:
		-$(SUITE)

valg:
		-VALG=Y $(SUITE)

time:
		-VALG=T $(SUITE)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:

uninstall:

clean:
		rm -f output/* tmp/* valg_log.txt log.txt *.o *.tmp
