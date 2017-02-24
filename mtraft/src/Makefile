include		../_conf.txt

OBJS		=		\
		$(TK_OBJ)	\
		be_busy.o	\
		be_misc.o	\
		be_scan.o	\


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CXX) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

$(OBJS):	*.h *.xpm

# .c.o Implicit rule
# .cpp.o Implicit rule

moc_tk_qt4.cpp:	tk_qt4.h
		$(MOC) tk_qt4.h -o $@

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f moc_*.cpp *.o $(BIN_NAME)
