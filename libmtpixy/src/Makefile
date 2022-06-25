include		../_conf.txt

HEADER		:= mtpixy.h mtpixy_cairo.h
OBJS		:= $(sort $(patsubst %.cpp,%.o,$(wildcard *.cpp)))
OBJS		:= $(sort $(patsubst %.c,%.o,$(wildcard *.c)) $(OBJS))

.PHONY:		all install uninstall clean


all:		$(LIB_SONAME)


$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$@

$(OBJS):	*.h

%.o:		%.cpp
		$(CXX) -I$(CURDIR) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o:		%.c
		$(CC) -I$(CURDIR) $(CPPFLAGS) $(CFLAGS) -c $<

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
		rm -f *.o *.so.$(LIB_SONUM)
