## Process this file with automake to produce Makefile.in

CPPFLAGS = -D_REENTRANT -D_LINUX -D_LITTLE_ENDIAN -D_UNIX

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) \
	$(REAL_INCLUDEDIR) \
	$(SDL_INCLUDEDIR) \
	-I/usr/include/gnome-xml


bin_PROGRAMS = coriander

coriander_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	definitions.c definitions.h \
	thread_base.c thread_base.h \
	thread_iso.c thread_iso.h \
	SDLEvent.c SDLEvent.h \
	thread_display.c thread_display.h \
	thread_ftp.c thread_ftp.h \
	thread_save.c thread_save.h \
	RealErrorSink.cc RealErrorSink.h \
	thread_real.cc thread_real.h \
	tools.c tools.h \
	callbacks.c callbacks.h \
	update_frames.c update_frames.h \
	build_frames.c build_frames.h \
	update_windows.c update_windows.h \
	build_windows.c build_windows.h \
	update_menus.c update_menus.h \
	build_menus.c build_menus.h \
	update_ranges.c update_ranges.h \
	build_ranges.c build_ranges.h \
	conversions.c conversions.h \
	topology.c topology.h \
	preferences.cc preferences.h

LDFLAGS += -Xlinker --export-dynamic

coriander_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(PTHREAD_LIB) \
	 $(FTP_LIB) $(REAL_LIB) $(SDL_LIB) -ldc1394_control -lraw1394 -lgthread


