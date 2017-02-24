include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= $(HEADER_NAME)
OBJS		=		\
		$(TK_OBJ)	\
		moc_private.o	\
		mtqex_bmenu.o	\
		mtqex_dialog.o	\
		mtqex_file.o	\
		mtqex_image.o	\
		mtqex_misc.o	\
		mtqex_prefwin.o	\


.PHONY:		all install uninstall clean

all:		$(LIB_SONAME)

$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $(LIB_SONAME) $(OBJS) $(LDFLAGS) -Wl,-soname,$(LIB_SONAME)

$(OBJS):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

moc_mtqex4.cpp:
		$(MOC) mtqex4.h -o $@

moc_mtqex5.cpp:
		$(MOC) mtqex5.h -o $@

moc_private.cpp:
		$(MOC) private.h -o $@

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
		rm -f moc_*.cpp *.o $(LIB_SONAME)
