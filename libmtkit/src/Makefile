include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtkit.h
OBJS		=			\
		mtkit_arg.o		\
		mtkit_cli.o		\
		mtkit_cliitem.o		\
		mtkit_clitab.o		\
		mtkit_file.o		\
		mtkit_mtstring.o	\
		mtkit_prefs.o		\
		mtkit_recent.o		\
		mtkit_string.o		\
		mtkit_time.o		\
		mtkit_tree.o		\
		mtkit_utf8.o		\
		mtkit_utree.o		\
		mtkit_zip.o		\


.PHONY:		all install uninstall clean

all:		$(LIB_SONAME)

$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $(LIB_SONAME) $(OBJS) $(LDFLAGS) -Wl,-soname,$(LIB_SONAME)

$(OBJS):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(LIB_INSTALL)
		install $(LIB_SONAME) $(DESTDIR)$(LIB_INSTALL)/
		cd $(DESTDIR)$(LIB_INSTALL); ln -sf $(LIB_SONAME) $(LIB_NAME)
		install -d $(DESTDIR)$(HEAD_INSTALL)
		install -m 644 $(HEADER) $(DESTDIR)$(HEAD_INSTALL)

uninstall:
		rm -f $(DESTDIR)$(LIB_INSTALL)/$(LIB_NAME)
		rm -f $(DESTDIR)$(LIB_INSTALL)/$(LIB_SONAME)
		rm -f $(DESTDIR)$(HEAD_INSTALL)/$(HEADER)

clean:
		rm -f *.o $(LIB_SONAME)
