include		../_conf.txt

HEADER		:= mtcelledit.h
PARSER		:= ced_parser.c
PARSER_REPORT	:= ced_parser_report.txt
OBJS		:= $(patsubst %.y,%.o,$(wildcard *.y))
OBJS		:= $(sort $(patsubst %.c,%.o,$(wildcard *.c)) $(OBJS))

.PHONY:		all install uninstall clean


all:		$(LIB_SONAME)


$(LIB_SONAME):	$(OBJS)
		$(CC) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$@

$(OBJS):	*.h

$(PARSER):	ced_parser.y
		bison $< -W --report=all --report-file=$(PARSER_REPORT) -o $@

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
		rm -f *.o *.so.$(LIB_SONUM) $(PARSER) $(PARSER_REPORT)
