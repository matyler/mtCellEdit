.PHONY:		all install uninstall clean

OBJS		=		\
		chap_00.html	\
		chap_01.html	\
		chap_02.html	\
		chap_03.html	\
		chap_04.html	\
		chap_05.html	\
		chap_A.html	\
		chap_B.html	\
		index.html	\


all:		$(OBJS)

.SUFFIXES:	.html .t2t

.t2t.html:
		txt2tags -t html -o $@ $<

install:

uninstall:

clean:
