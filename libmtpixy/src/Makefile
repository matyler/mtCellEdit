include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtpixy.h
OBJS		=		\
		px_blit.o	\
		px_brush.o	\
		px_convert.o	\
		px_effects.o	\
		px_file.o	\
		px_file_bmp.o	\
		px_file_gif.o	\
		px_file_jpeg.o	\
		px_file_png.o	\
		px_floodfill.o	\
		px_font.o	\
		px_image.o	\
		px_impal.o	\
		px_overlay.o	\
		px_paint.o	\
		px_palette.o	\
		px_paste.o	\
		px_polygon.o	\
		px_quantize.o	\
		px_scale.o	\
		px_text.o	\


.PHONY:		all install uninstall clean

all:		$(LIB_SONAME)

$(LIB_SONAME):	$(OBJS)
		$(CXX) -shared -o $(LIB_SONAME) $(OBJS) $(LDFLAGS) -Wl,-soname,$(LIB_SONAME)

$(OBJS):	*.h

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
		rm -f *.o $(LIB_SONAME)
