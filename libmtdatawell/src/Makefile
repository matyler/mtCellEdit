include		../_conf.txt

HEADER		:= mtdatawell.h mtdatawell_math.h mtdatawell_sqlite.h
OBJS		:= $(sort $(patsubst %.cpp,%.o,$(wildcard *.cpp)))
OBJS_GPERF	:= $(sort $(patsubst %.gperf,%.h,$(wildcard *.gperf)))


.PHONY:		all install uninstall clean


all:		$(LIB_SONAME)


$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$@

$(OBJS):	*.h $(OBJS_GPERF)


%.o:		%.cpp
		$(CXX) -I$(CURDIR) $(CPPFLAGS) $(CXXFLAGS) -c $<

%.o:		%.c
		$(CC) -I$(CURDIR) $(CPPFLAGS) $(CFLAGS) -c $<

%.h:		%.gperf
		gperf -D $< --output-file=$@

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
		rm -f *.o *.so.$(LIB_SONUM) $(OBJS_GPERF)
