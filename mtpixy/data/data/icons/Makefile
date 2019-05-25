OBJS		=		\
		01.png		\
		02.png		\
		03.png		\
		04.png		\
		05.png		\
		06.png		\
		07.png		\
		08.png		\
		09.png		\
		10.png		\

.PHONY:		all install uninstall clean
.NOTPARALLEL:

all:		$(OBJS)

$(OBJS):	default.svg
		./build.sh

install:

uninstall:

clean:
		rm -rf *.png
