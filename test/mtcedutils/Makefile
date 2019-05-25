include		../_conf.txt


.PHONY:		all valg time install uninstall clean


TMP_DIR		= output
TMP_FILES	= valg_log.txt log.txt
PREPARE		= mkdir -p $(TMP_DIR)


all:
		$(PREPARE)
		$(SUITE)

valg:
		$(PREPARE)
		VALG=Y $(SUITE)

time:
		$(PREPARE)
		VALG=T $(SUITE)

install:

uninstall:

clean:
		rm -rf $(TMP_DIR) $(TMP_FILES)

