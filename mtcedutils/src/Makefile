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
OBJS_CED	:= $(sort $(patsubst %.cpp,%.o,$(wildcard ced*.cpp)) $(OBJS_CED))
OBJS_RDC	:= $(sort $(patsubst %.c,%.o,$(wildcard rdc*.c)))
OBJS_RDC	:= $(sort $(patsubst %.cpp,%.o,$(wildcard rdc*.cpp)) $(OBJS_RDC))
BINS		:= cedutils mtrdc mtnetlog

.PHONY:		all install uninstall clean


all:		$(BINS)


cedutils:	$(OBJS_CED)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_CED):	ced*.h

mtrdc:		$(OBJS_RDC)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_RDC):	rdc.h

mtnetlog:	mtnetlog.o
		$(CXX) $^ -o $@ $(LDFLAGS)

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
