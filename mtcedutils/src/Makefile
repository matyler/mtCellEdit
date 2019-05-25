include		../_conf.txt

APPS		:=		\
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

OBJS_CED	:= $(sort $(patsubst %.c,%.o,$(wildcard ced*.c)))
OBJS_RDC	:= $(sort $(patsubst %.c,%.o,$(wildcard rdc*.c)))
BINS		:= cedutils mtrdc mtnetlog

.PHONY:		all install uninstall clean


all:		$(BINS)


cedutils:	$(OBJS_CED)
		$(CC) $^ -o $@ $(LDFLAGS)

$(OBJS_CED):	ced.h ced_fuzzmap.h

mtrdc:		$(OBJS_RDC)
		$(CC) $^ -o $@ $(LDFLAGS)

$(OBJS_RDC):	rdc.h

mtnetlog:	mtnetlog.o
		$(CC) $^ -o $@ $(LDFLAGS)

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
