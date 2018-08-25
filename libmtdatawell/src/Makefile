include		../_conf.txt

LIB_SONAME	= $(LIB_NAME).1
HEADER		= mtdatawell.h
OBJS		=			\
		butt_databutt.o		\
		butt_op.o		\
		butt_otp.o		\
		butt_read.o		\
		core_misc.o		\
		soda_datasoda.o		\
		soda_decode.o		\
		soda_encode.o		\
		soda_load.o		\
		soda_op.o		\
		tap_audio.o		\
		tap_audio_decode.o	\
		tap_audio_encode.o	\
		tap_datatap.o		\
		tap_file.o		\
		tap_image_decode.o	\
		tap_image_encode.o	\
		tap_op.o		\
		well_datawell.o		\
		well_filedb.o		\
		well_filescan.o		\
		well_filestream.o	\
		well_op.o		\
		well_save.o		\


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
