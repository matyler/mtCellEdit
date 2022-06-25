include		../_conf.txt

HEADER		:= $(HEADER_NAME)
OBJS		:= $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS		:= $(sort $(TK_OBJ) moc_private.o $(OBJS))

.PHONY:		all install uninstall clean
.SECONDARY:


all:		$(LIB_SONAME)


$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$@

$(OBJS):	$(HEADER) private.h

# .c.o Implicit rule
# .cpp.o Implicit rule

# Pass 1 checks target .h validity, pass 2 creates .h file
mtqex%.h:	mtqex.h
		./build_header.sh $@ < $< > /dev/null
		./build_header.sh $@ < $< > $@

moc_%.cpp:	%.h
		$(MOC) $< -o $@

install:
		install -d $(DESTDIR)$(LIB_INSTALL)
		install $(LIB_SONAME) $(DESTDIR)$(LIB_INSTALL)/
		cd $(DESTDIR)$(LIB_INSTALL); ln -sf $(LIB_SONAME) $(LIB_NAME)
		install -d $(DESTDIR)$(HEAD_INSTALL)
		install -m 644 $(HEADER) $(DESTDIR)$(HEAD_INSTALL)

uninstall:
		rm -f $(DESTDIR)$(LIB_INSTALL)/$(LIB_NAME)
		rm -f $(DESTDIR)$(LIB_INSTALL)/$(LIB_SONAME)
		cd $(DESTDIR)$(HEAD_INSTALL); rm -f $(HEADER)

clean:
		rm -f moc_*.cpp *.o *.so.$(LIB_SONUM) mtqex4.h mtqex5.h
