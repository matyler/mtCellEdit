include		../_conf.txt

OBJS_BE		=		\
		be_cline.o	\
		be_main.o	\
		be_prefs.o	\

OBJS_QT		=			\
		moc_qt45.o		\
		qt45_canvas.o		\
		qt45_d_brush.o		\
		qt45_d_misc.o		\
		qt45_d_palette.o	\
		qt45_d_resize.o		\
		qt45_d_scale.o		\
		qt45_d_text.o		\
		qt45_d_transcolor.o	\
		qt45_dock.o		\
		qt45_edit.o		\
		qt45_file.o		\
		qt45_img_eff.o		\
		qt45_keyboard.o		\
		qt45_main.o		\
		qt45_menu.o		\
		qt45_mouse.o		\
		qt45_options.o		\
		qt45_palette.o		\
		qt45_prefs.o		\
		qt45_selection.o	\
		qt45_statusbar.o	\
		qt45_tool.o		\


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

mtpixy-qt4:	$(OBJS_BE) $(OBJS_QT)
		$(CXX) $(OBJS_BE) $(OBJS_QT) -o $@ $(LDFLAGS)

mtpixy-qt5:	$(OBJS_BE) $(OBJS_QT)
		$(CXX) $(OBJS_BE) $(OBJS_QT) -o $@ $(LDFLAGS)

$(OBJS_BE):	be*.h

$(OBJS_QT):	qt45*.h be*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

moc_qt45.cpp:	qt45.h
		$(MOC) qt45.h -o $@

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f moc_*.cpp *.o $(BIN_NAME)
