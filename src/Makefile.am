## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) \
	-I/usr/include/gnome-xml

bin_PROGRAMS = coriander

coriander_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	update_frames.c update_frames.h \
	build_frames.c build_frames.h \
	update_windows.c update_windows.h \
	build_windows.c build_windows.h \
	update_menus.c update_menus.h \
	build_menus.c build_menus.h \
	update_ranges.c update_ranges.h \
	build_ranges.c build_ranges.h \
	callback_proc.c callback_proc.h \
	definitions.c definitions.h \
	conversions.c conversions.h \
	capture.c capture.h \
	tools.c tools.h \
	topology.c topology.h

coriander_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(PTHREAD_LIB) \
	$(XV_LIB) $(FTP_LIB) -ldc1394_control -lraw1394 -lgthread 

