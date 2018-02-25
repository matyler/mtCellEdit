include		../_conf.txt

OBJS		=		\
		be_cellprefs.o	\
		be_cline.o	\
		be_clipboard.o	\
		be_find.o	\
		be_graph.o	\
		be_misc.o	\
		be_prefs.o	\

OBJS_QT4	= $(OBJS)	\
		moc_qt4.o	\
		qt4_clipboard.o	\
		qt4_dialogs.o	\
		qt4_edit.o	\
		qt4_file.o	\
		qt4_find.o	\
		qt4_graph.o	\
		qt4_input.o	\
		qt4_main.o	\
		qt4_menu.o	\
		qt4_prefs.o	\
		qt4_sheet.o	\
		qt4_sort.o	\
		qt4_updates.o	\
		qt4_view.o	\

OBJS_QT5	= $(OBJS_QT4)


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

mtcelledit-qt4:	$(OBJS_QT4)
		$(CXX) $(OBJS_QT4) -o $@ $(LDFLAGS)

mtcelledit-qt5:	$(OBJS_QT5)
		$(CXX) $(OBJS_QT5) -o $@ $(LDFLAGS)

$(OBJS_QT4) $(OBJS_QT5):	*.h *.xpm

# .c.o Implicit rule
# .cpp.o Implicit rule

moc_qt4.cpp:	qt4.h
		$(MOC) qt4.h -o $@

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f moc_*.cpp *.o $(BIN_NAME)
