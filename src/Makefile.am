## Process this file with automake to produce Makefile.in

# CPPFLAGS = 

INCLUDES = -I$(top_srcdir)/intl $(GNOME_INCLUDEDIR) $(SDLLIB_CFLAGS) $(LIBRAW_CFLAGS) $(LIBDC_CFLAGS) $(GDK_PIXBUF_CFLAGS) $(COR_LFS_CFLAGS) -I/usr/include/gnome-xml

bin_PROGRAMS = coriander

coriander_SOURCES = \
	main.c \
	coriander.h \
	support.c support.h \
	interface.c interface.h \
	definitions.c definitions.h \
	camera.c camera.h \
	thread_base.c thread_base.h \
	thread_iso.c thread_iso.h \
	watch_thread.c watch_thread.h \
	SDLEvent.c SDLEvent.h \
	thread_display.c thread_display.h \
	thread_ftp.c thread_ftp.h \
	thread_save.c thread_save.h \
	thread_v4l.c thread_v4l.h videodev.h \
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
	preferences.c preferences.h \
	raw1394support.h \
	icons.h

#$(INTLLIBS) removed from the line below
coriander_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(PTHREAD_LIB) $(FTPLIB_LIBS) $(SDLLIB_LIBS) $(LIBDC_LIBS) $(LIBRAW_LIBS) $(GDK_PIXBUF_LIBS) $(XV_LIBS) $(COR_LFS_LDFLAGS) -lgthread 


