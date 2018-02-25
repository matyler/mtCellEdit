include		../_conf.txt

APPS		=		\
		pixydelta	\
		pixyls		\
		pixynew		\
		pixypica	\
		pixyresize	\
		pixyriba	\
		pixyrisa	\
		pixyscale	\
		pixytwit	\

OBJS		=		\
		func_misc.o	\
		func_pica.o	\
		func_riba.o	\
		func_rida.o	\
		func_risa.o	\
		func_twit.o	\
		main.o		\


.PHONY:		all install uninstall clean

all:		$(BIN_NAME)

$(BIN_NAME):	$(OBJS)
		$(CXX) $(OBJS) -o $(BIN_NAME) $(LDFLAGS)

$(OBJS):	*.h

# .c.o Implicit rule
# .cpp.o Implicit rule

install:
		install -d $(DESTDIR)$(BIN_INSTALL)
		install $(BIN_NAME) $(DESTDIR)$(BIN_INSTALL)
		cd $(DESTDIR)$(BIN_INSTALL); for APP in $(APPS); do ln -sfv $(BIN_NAME) $$APP; done

uninstall:
		cd $(DESTDIR)$(BIN_INSTALL); rm $(BIN_NAME) $(APPS)

clean:
		rm -f *.o $(BIN_NAME)
