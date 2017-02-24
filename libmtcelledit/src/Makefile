include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtcelledit.h
PARSER		= ced_parser.cpp
PARSER_REPORT	= ced_parser_report.txt
OBJS		=		\
		ced_book.o	\
		ced_cell.o	\
		ced_file.o	\
		ced_find.o	\
		ced_index.o	\
		ced_output.o	\
		ced_parser.o	\
		ced_paste.o	\
		ced_sheet.o	\
		ced_sort.o	\
		ced_token.o	\
		ced_transform.o	\
		ced_utils.o	\


.PHONY:		all install uninstall clean

all:		$(LIB_SONAME)

$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $(LIB_SONAME) $(OBJS) $(LDFLAGS) -Wl,-soname,$(LIB_SONAME)

$(OBJS):	*.h

$(PARSER):	ced_parser.yy ced_parser_def.cpp
		bison ced_parser.yy -W --report=all --report-file=$(PARSER_REPORT) -o $(PARSER)

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
		rm -f *.o $(LIB_SONAME) $(PARSER) $(PARSER_REPORT)
