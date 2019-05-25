include		../_conf.txt


.PHONY:		all valg time install uninstall clean


# NOTE: - at the beginning of the instruction to ignore errors

TMP_DIR		= output tmp
TMP_FILES	= valg_log.txt log.txt
PREPARE		= mkdir -p $(TMP_DIR)


all:
		$(PREPARE)
		-$(SUITE)

valg:
		$(PREPARE)
		-VALG=Y $(SUITE)

time:
		$(PREPARE)
		-VALG=T $(SUITE)


# .c.o Implicit rule
# .cpp.o Implicit rule


install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

