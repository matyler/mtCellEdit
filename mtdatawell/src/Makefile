include		../_conf.txt

OBJS_BE		:= $(patsubst %.cpp,%.o,$(wildcard be*.cpp))
OBJS_MOC	:= $(patsubst %.h,moc_%.o,$(wildcard qt4*.h))
OBJS_QT4	:= $(patsubst %.cpp,%.o,$(wildcard qt4*.cpp))
OBJS_QT4	:= $(sort $(OBJS_BE) $(OBJS_MOC) $(OBJS_QT4))
OBJS_QT5	:= $(OBJS_QT4)

.PHONY:		all install uninstall clean


all:		$(BIN_NAME)


mtdatawell-qt4:	$(OBJS_QT4)
		$(CXX) $^ -o $@ $(LDFLAGS)

mtdatawell-qt5:	$(OBJS_QT5)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_QT4):	*.h

$(OBJS_QT5):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

moc_%.cpp:	%.h
		$(MOC) $< -o $@

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)

uninstall:
		rm $(DESTDIR)$(BIN_INSTALL)/$(BIN_NAME)

clean:
		rm -f moc_*.cpp *.o $(BIN_NAME)
