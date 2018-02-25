include		../_conf.txt

OBJS		=		\
		be_cline.o	\
		be_main.o	\
		be_prefs.o	\

OBJS_QT4	= $(OBJS)		\
		moc_qt4.o		\
		qt4_canvas.o		\
		qt4_d_brush.o		\
		qt4_d_misc.o		\
		qt4_d_palette.o		\
		qt4_d_resize.o		\
		qt4_d_scale.o		\
		qt4_d_text.o		\
		qt4_d_transcolor.o	\
		qt4_dock.o		\
		qt4_edit.o		\
		qt4_file.o		\
		qt4_img_eff.o		\
		qt4_keyboard.o		\
		qt4_main.o		\
		qt4_menu.o		\
		qt4_mouse.o		\
		qt4_options.o		\
		qt4_palette.o		\
		qt4_prefs.o		\
		qt4_selection.o		\
		qt4_statusbar.o		\
		qt4_tool.o		\

OBJS_QT5	= $(OBJS_QT4)


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

mtpixy-qt4:	$(OBJS_QT4)
		$(CXX) $(OBJS_QT4) -o $@ $(LDFLAGS)

mtpixy-qt5:	$(OBJS_QT5)
		$(CXX) $(OBJS_QT5) -o $@ $(LDFLAGS)

$(OBJS_QT4) $(OBJS_QT5):	*.h

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
