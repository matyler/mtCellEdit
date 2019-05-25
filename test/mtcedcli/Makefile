include		../_conf.txt


.PHONY:		all valg time install uninstall clean


TMP_DIR		= output tmp
TMP_FILES	= valg_log.txt log.txt *.tmp
PREPARE		= mkdir -p $(TMP_DIR)


# NOTE: - at the beginning of the instruction to ignore errors

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

