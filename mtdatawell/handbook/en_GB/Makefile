.PHONY:		all install uninstall clean

OBJS		:= $(sort $(patsubst %.t2t,%.html,$(wildcard *.t2t)))


all:		$(OBJS)


%.html:		%.t2t
		txt2tags -t html -o $@ $<

install:

uninstall:

clean:
		rm -f *.html
