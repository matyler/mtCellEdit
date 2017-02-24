include		../_conf.txt

APPS		=		\
		cedappend	\
		cedclear	\
		cedcut		\
		ceddiff		\
		cedeval		\
		cedfind		\
		cedflip		\
		cedinsert	\
		cedls		\
		cedpaste	\
		cedrotate	\
		cedset		\
		cedsort		\
		cedtranspose	\

OBJS		=		\
		func_diff.o	\
		func_misc.o	\
		func_sort.o	\
		main.o		\


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CXX) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

$(OBJS):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)
		cd $(DESTDIR)$(BIN_INSTALL); for APP in $(APPS); do ln -sfv $(BIN_NAME) $$APP; done

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL); rm $(BIN_NAME) $(APPS)

clean:
		rm -f *.o $(BIN_NAME)
