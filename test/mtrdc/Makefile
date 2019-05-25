include		../_conf.txt


.PHONY:		all valg time install uninstall clean


TMP_DIR		= tmp
TMP_FILES	= valg_log.txt log.txt
PREPARE		= mkdir -p $(TMP_DIR)
CLEAN		= rm -rf $(TMP_DIR) $(TMP_FILES)


all:
		$(CLEAN)
		$(PREPARE)
		$(SUITE)

valg:
		$(CLEAN)
		$(PREPARE)
		VALG=Y $(SUITE)

time:
		$(CLEAN)
		$(PREPARE)
		VALG=T $(SUITE)

install:

uninstall:

clean:
		$(CLEAN)
