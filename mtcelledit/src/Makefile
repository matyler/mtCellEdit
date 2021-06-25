include		../_conf.txt

OBJS_BE		:= $(patsubst %.cpp,%.o,$(wildcard be*.cpp))
OBJS_MOC	:= $(patsubst %.h,moc_%.o,$(wildcard qt5*.h))
OBJS_QT5	:= $(patsubst %.cpp,%.o,$(wildcard qt5*.cpp))
OBJS_QT5	:= $(sort $(OBJS_BE) $(OBJS_MOC) $(OBJS_QT5))
OBJS_QT6	:= $(OBJS_QT5)

.PHONY:		all install uninstall clean


all:		$(BIN_NAME)


mtcelledit-qt5:	$(OBJS_QT5)
		$(CXX) $^ -o $@ $(LDFLAGS)

mtcelledit-qt6:	$(OBJS_QT6)
		$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJS_QT5):	*.h

$(OBJS_QT6):	*.h

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
