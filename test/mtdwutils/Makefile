include		../_conf.txt


.PHONY:		all valg time install uninstall clean


TMP_DIR		=	db				\
			soda_a soda_tmp soda_b		\
			bottle_a bottle_tmp bottle_b	\
			hg_a hg_tmp hg_b		\
			font_a font_tmp font_b

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

