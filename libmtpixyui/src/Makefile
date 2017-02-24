include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtpixyui.h
OBJS		=		\
		pui_clipboard.o	\
		pui_effects.o	\
		pui_file.o	\
		pui_mask.o	\
		pui_palette.o	\
		pui_render.o	\
		pui_tool.o	\
		pui_undo.o	\


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
