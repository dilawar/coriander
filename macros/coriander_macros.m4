dnl
dnl Coriander custom macros for system check.
dnl (C) 2002 by Damien Douxchamps
dnl


AC_DEFUN([AC_CHECK_LIBDC],[
	AC_MSG_CHECKING(for libdc1394)
 	AC_TRY_CPP([
	#include <libdc1394/dc1394_control.h>
	],[
 	libdc1394=yes
 	],[
 	libdc1394=no
 	])
 	AC_MSG_RESULT($libdc1394)
 	if test x$libdc1394 = xno; then
          AC_ERROR([libdc1394 is not installed.  
**************************************************************************
**   Please install libdc1394 version >= 0.9.3                          **
**   Source tarball and CVS at:  http://www.sf.net/projects/libdc1394   **
**************************************************************************])
 	fi
]) 

AC_DEFUN([AC_CHECK_LIBDC_VERSION],[
	AC_SUBST(LIBDC_CFLAGS)
	AC_SUBST(LIBDC_LIBS)
 	AC_CHECK_LIB(dc1394_control,GetCameraControlRegister,libdc1394=ok,libdc1394=old,-lraw1394)
 	if test x$libdc1394 = xok; then
           LIBDC_LIBS="-ldc1394_control"
           LIBDC_CFLAGS=""
	else
          AC_ERROR([libdc1394 is too old. 
**************************************************************************
**   Please upgrade to the current CVS or to a version >= 0.9.2         **
**   Source tarball and CVS at:  http://www.sf.net/projects/libdc1394   **
**************************************************************************])
 	fi
])

AC_DEFUN([AC_CHECK_LIBRAW],[
	AC_MSG_CHECKING(for libraw1394)
 	AC_TRY_CPP([
	#include <libraw1394/raw1394.h>
 	],[
 	libraw1394=yes
 	],[
 	libraw1394=no
 	])
 	AC_MSG_RESULT($libraw1394)
 	if test x$libraw1394 = xno; then
          AC_ERROR([libraw1394 is not installed.
**************************************************************************
**   Please install version >= 0.9.0                                    **
**   Source tarball and CVS at:  http://www.sf.net/projects/libraw1394  **
**************************************************************************])
	fi
])

AC_DEFUN([AC_CHECK_LIBRAW_VERSION],[
	AC_SUBST(LIBRAW_CFLAGS)
	AC_SUBST(LIBRAW_LIBS)
 	AC_CHECK_LIB(raw1394,raw1394_new_handle,libraw1394=ok,libraw1394=old)
 	if test x$libraw1394 = xok; then
           LIBRAW_LIBS="-lraw1394"
           LIBRAW_CFLAGS=""
	else
          AC_ERROR([libraw1394 is too old.
**************************************************************************
**   Please upgrade to a version >= 0.9.0                               **
**   Source tarball and CVS at:  http://www.sf.net/projects/libraw1394  **
**************************************************************************])
	fi
])


AC_DEFUN([AC_CHECK_FTPLIB],[
	AC_SUBST(FTPLIB_CFLAGS)
	AC_SUBST(FTPLIB_LIBS)
	AC_CHECK_LIB(ftp, FtpInit,
          AC_DEFINE(HAVE_FTPLIB)
          FTPLIB_LIBS="-lftp"
          FTPLIB_CFLAGS="",
          AC_MSG_RESULT([ftplib is required for FTP support.
**************************************************************************
**   Source tarball available at: http://nbpfaus.net/~pfau/ftplib/      **
**   FTP SERVICE DISABLED                                               **
**************************************************************************]))
])

AC_DEFUN([AC_CHECK_SDLLIB],[
	AC_SUBST(SDLLIB_CFLAGS)
	AC_SUBST(SDLLIB_LIBS)
	AC_CHECK_PROG(have_sdl_config, sdl-config, "found", "not found")
	if test x$have_sdl_config = xfound; then
	  SDLLIB_LIBS=`sdl-config --libs`
	  SDLLIB_CFLAGS=`sdl-config --cflags`
	  AC_DEFINE(HAVE_SDLLIB)
	else
	  AC_MSG_RESULT([SDL required for display support.
**************************************************************************
**   SDL can be downloaded in various formats at http://www.libsdl.org  **
**   DISPLAY SERVICE DISABLED                                           **
**************************************************************************])
	fi
])

AC_DEFUN([AC_CHECK_MMX],[
	AC_MSG_CHECKING(for MMX support)
 	mmx=no
 	AC_TRY_RUN([
	int
	main (int argc, char **argv)
	{
	asm("movq %mm0, %mm1");// check basic MMX functions
  	asm("emms");
	exit(0);
	}	
 	],[
 	mmx=yes
 	],[
 	mmx=no
 	],[
 	mmx=no
 	])
 	AC_MSG_RESULT($mmx)
 	if test x$mmx = xyes; then
  	   AC_DEFINE(MMX_SUPPORT)
 	fi

])

AC_DEFUN([AC_CHECK_SSE],[
	AC_MSG_CHECKING(for SSE support)
 	sse=no
 	AC_TRY_RUN([
	int
	main (int argc, char **argv)
	{
	asm("xorps %xmm0, %xmm0");// check SSE floating point
	asm("pmulhuw %mm1, %mm0");// check SSE additional integer ('MMX+')
	exit(0);
	}
 	],[
	sse=yes
 	],[
	sse=no
 	],[
	sse=no
 	])
 	AC_MSG_RESULT($sse)
 	if test x$sse = xyes; then
  	   AC_DEFINE(SSE_SUPPORT)
 	fi

])


AC_DEFUN([AC_CHECK_SSE2],[
	AC_MSG_CHECKING(for SSE2 support)
 	sse2=no
 	AC_TRY_RUN([
	int
	main (int argc, char **argv)
	{
	asm("movapd %xmm0, %xmm1");// check SSE double-precision floating point
	asm("pmuludq %xmm0, %xmm1");
	asm("pand %xmm0, %xmm1");// check SSE 128-bit MMX extension
	exit(0);
	}
 	],[
 	sse2=yes
 	],[
 	sse2=no
 	],[
 	sse2=no
 	])
 	AC_MSG_RESULT($sse2)
 	if test x$sse2 = xyes; then
  	   AC_DEFINE(SSE2_SUPPORT)
 	fi

])


AC_DEFUN([AC_CHECK_GDK_PIXBUF],[
	AC_SUBST(GDK_PIXBUF_CFLAGS)
	AC_SUBST(GDK_PIXBUF_LIBS)
	AC_CHECK_PROG(have_gdk_pixbuf_config, gdk-pixbuf-config, "found", "not found")
	if test x$have_gdk_pixbuf_config = xfound; then
	  GDK_PIXBUF_LIBS=`gdk-pixbuf-config --libs`
	  GDK_PIXBUF_CFLAGS=`gdk-pixbuf-config --cflags`
	  AC_DEFINE(HAVE_GDK_PIXBUF)
	else
	  AC_MSG_RESULT([GDK-pixbuf is required for WM icons.])
	fi
])

AC_DEFUN([AC_CHECK_XV],[
	AC_SUBST(XV_CFLAGS)
	AC_SUBST(XV_LIBS)
	AC_MSG_CHECKING(for Xv extension)
	AC_TRY_COMPILE([
	#include <X11/Xlib.h>
 	#include <X11/extensions/Xvlib.h>],[
	int main(void) { (void) XvGetPortAttribute(0, 0, 0, 0); return 0; }
	],xv=ok,xv=no);
	AC_MSG_RESULT($xv)
 	if test x$xv = xok; then
           XV_LIBS="-lXv"
           XV_CFLAGS=""
	   AC_DEFINE(HAVE_XV)
	else
          AC_ERROR([The XV overlay libraries were not found 
**************************************************************************
**   Please correct your X11 installation. Run 'xvinfo' to check if     **
**   your system has overlay capabilities.                              **
**************************************************************************])
 	fi
])