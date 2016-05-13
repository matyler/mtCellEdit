include		../_conf.txt

CFLAGS		= $(LIB_CFLAGS) $(CFLAG)
LDFLAGS 	= $(LDFLAG)
LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtgex.h
OBJS		=		\
		mtgex_bmenu.o	\
		mtgex_cpick.o	\
		mtgex_fpick.o	\
		mtgex_general.o	\
		mtgex_image.o	\
		mtgex_prefs.o	\


.PHONY: 	all install uninstall clean

all:		$(LIB_SONAME)

$(LIB_SONAME):	$(OBJS)
		$(CC) -shared -o $(LIB_SONAME) $(OBJS) $(LDFLAGS) -Wl,-soname,$(LIB_SONAME)

$(OBJS):	*.h graphics/*

# .c.o Implicit rule

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
