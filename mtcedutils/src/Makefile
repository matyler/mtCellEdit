include		../_conf.txt

APPS		=		\
		cedappend	\
		cedclear	\
		cedcut		\
		ceddiff		\
		cedeval		\
		cedfind		\
		cedflip		\
		cedfuzzmap	\
		cedinsert	\
		cedls		\
		cedpaste	\
		cedrotate	\
		cedset		\
		cedsort		\
		cedtranspose	\

OBJS_CED	=		\
		ced_diff.o	\
		ced_fuzzmap.o	\
		ced_main.o	\
		ced_misc.o	\
		ced_sort.o	\

OBJS_RDC	=		\
		rdc_analysis.o	\
		rdc_main.o	\
		rdc_matrix.o	\
		rdc_pad.o	\
		rdc_password.o	\
		rdc_prng.o	\
		rdc_shuffle.o	\
		rdc_xor.o	\

BINS		= cedutils mtrdc mtnetlog

.PHONY:		all install uninstall clean

all:		$(BINS)

cedutils:	$(OBJS_CED)
		$(CC) $(OBJS_CED) -o $@ $(LDFLAGS)

$(OBJS_CED):	ced.h ced_fuzzmap.h

mtrdc:		$(OBJS_RDC)
		$(CC) $(OBJS_RDC) -o $@ $(LDFLAGS)

$(OBJS_RDC):	rdc.h

mtnetlog:	mtnetlog.o
		$(CC) mtnetlog.o -o $@ $(LDFLAGS)

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BINS) $(DESTDIR)$(BIN_INSTALL)
		cd $(DESTDIR)$(BIN_INSTALL); for APP in $(APPS); do ln -sfv cedutils $$APP; done

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL); rm $(BINS) $(APPS)

clean:
		rm -f *.o $(BINS)
