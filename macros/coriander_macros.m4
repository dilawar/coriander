dnl
dnl Coriander custom macros for system check.
dnl (C) 2002 by Damien Douxchamps
dnl


AC_DEFUN([AC_CHECK_LIBDC],[
	AC_SUBST(LIBDC_CFLAGS)
	AC_SUBST(LIBDC_LIBS)

	AC_MSG_CHECKING(for libdc1394)
 	AC_TRY_COMPILE([
	#include <libdc1394/dc1394_control.h>
	#include <libraw1394/raw1394.h>
 	],[
        raw1394handle_t handle;
        dc1394_camerainfo info;
	dc1394_cameracapture capture;
	int value;
  	dc1394_get_sw_version(info.handle, info.id, &value);
	capture.dma_device_file=NULL;
 	],[
 	libdc1394=yes
 	])
 	AC_MSG_RESULT($libdc1394)
 	if test x$libdc1394 = xyes; then
           LIBDC_LIBS="-ldc1394_control"
           LIBDC_CFLAGS=""
	else
          AC_ERROR(libdc1394 is not installed or is too old. Please update at least to CVS 2002-02-01)
 	fi
dnl	AC_CHECK_LIB(dc1394_control, dc1394_get_sw_version,
dnl          LIBDC_LIBS="-ldc1394_control"
dnl          LIBDC_CFLAGS="",
dnl          AC_ERROR(libdc1394 is not installed or is too old.))
])

AC_DEFUN([AC_CHECK_LIBRAW],[
	AC_SUBST(LIBRAW_CFLAGS)
	AC_SUBST(LIBRAW_LIBS)
	AC_CHECK_LIB(raw1394, raw1394_new_handle,
          LIBRAW_LIBS="-lraw1394"
          LIBRAW_CFLAGS="",
          AC_ERROR(libraw1394 is not installed or is too old.))
])

AC_DEFUN([AC_CHECK_FTPLIB],[
	AC_SUBST(FTPLIB_CFLAGS)
	AC_SUBST(FTPLIB_LIBS)
	AC_CHECK_LIB(ftp, FtpInit,
          AC_DEFINE(HAVE_FTPLIB)
          FTPLIB_LIBS="-lftp"
          FTPLIB_CFLAGS="",
          AC_MSG_RESULT(ftplib is required for ftp support. FTP SERVICE WILL NOT BE AVAILABLE))
])

AC_DEFUN([AC_CHECK_SDLLIB],[
	AC_SUBST(SDLLIB_CFLAGS)
	AC_SUBST(SDLLIB_LIBS)
	AC_CHECK_PROG(have_sdl_config, sdl-config, "found", "not found")
	if test "x$have_sdl_config" = "xfound"; then
	  SDLLIB_LIBS=`sdl-config --libs`
	  SDLLIB_CFLAGS=`sdl-config --cflags`
	  AC_DEFINE(HAVE_SDLLIB)
	else
	  AC_MSG_RESULT(SDL required for display support. DISPLAY SERVICE WILL NOT BE AVAILABLE.)
	fi
])

AC_DEFUN([AC_CHECK_REALLIB],[
	AC_SUBST(REALLIB_CFLAGS)
	AC_SUBST(REALLIB_LIBS)
	AC_MSG_CHECKING(for Real Producer SDK)
 	real_sdk=no
 	AC_TRY_COMPILE([
	#include <realproducersdk/pncom.h>
	#include <realproducersdk/rmaerror.h>
	#include <realproducersdk/pnwintyp.h>
	#include <realproducersdk/engtypes.h>
	#include <realproducersdk/rmaenum.h>
	#include <realproducersdk/rmapckts.h>
	#include <realproducersdk/engstats.h>
	#include <realproducersdk/engcodec.h>
 	],[
  	IRMABuffer *pBuffer;
 	],[
 	real_sdk=yes
 	])
 	AC_MSG_RESULT($real_sdk)
 	if test x$real_sdk = xyes; then
  	   AC_DEFINE(HAVE_REALLIB)	
  	   REALLIB_LIBS="-L/usr/local/lib/realproducersdk -lenceng -ldl"
  	   REALLIB_CFLAGS="-I/usr/local/include/realproducersdk -D_REENTRANT -D_LINUX -D_LITTLE_ENDIAN -D_UNIX"
	else
	  AC_MSG_RESULT(RealProducerSDK required for Real streaming support. REAL SERVICE WILL NOT BE AVAILABLE.) 
 	fi

])

