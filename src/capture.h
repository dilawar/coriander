/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
 * Iso video receive, video overlay, and catpure provided by 
 * Dan Dennedy <dan@dennedy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <libraw1394/raw1394.h>

#define DMA_BUFFERS 10
#define MAX_FRAMES 10000

#define GUID_YUV12_PLANAR 0x32315659
#define GUID_YUY2_PACKED 0x32595559
#define GUID_UYVY_PACKED 0x59565955

typedef enum
{
  RECEIVE_METHOD_RAW1394,
  RECEIVE_METHOD_VIDEO1394
} receive_method_t;

typedef enum
{
  DISPLAY_METHOD_XV,
  DISPLAY_METHOD_GDK
} display_method_t;

typedef struct
{
  gboolean                is_open;
  GtkWidget              *drawable;
  display_method_t        display_method;
  pthread_t               thread;
  pthread_mutex_t         mutex;
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
  Display                *display;
  Window                  window;
  unsigned long           xv_format;
  XvImage                *xv_image;
  int                     xv_port;
  XShmSegmentInfo         xv_shm_info;
  GC                      xv_gc;
#endif
  char                   *gdk_buffer;
} porthole_info;

typedef struct
{
  pthread_mutex_t mutex_capture;
  pthread_mutex_t mutex_cancel_display;
  int cancel_display_thread;
  int porthole;
  int isothread;
  int capture;

} activity_info;

/*
typedef struct 
 {
      0,
      CAPTURE_FREQ_IMMEDIATE,
      1,
      CAPTURE_MODE_SEQUENTIAL,
      FALSE,
      "",
      "",
      "",
      ""
} ;

typedef struct
{ 
  pthread_t               thread;
  pthread_mutex_t         mutex;
  cap_data_t              data;
} capture_info;
*/

typedef struct
{ 
  pthread_t               thread;
  raw1394handle_t         handle;
  short                   buffer_updated;
  receive_method_t        receive_method;
} isothread_info;

typedef enum
{
  CAPTURE_FREQ_IMMEDIATE,
  CAPTURE_FREQ_PERIODIC
} capture_frequency;

typedef enum
{
  CAPTURE_MODE_SEQUENTIAL,
  CAPTURE_MODE_OVERWRITE
} capture_mode;

typedef struct
{
  pthread_t               thread;
  pthread_mutex_t         mutex;
  unsigned long           counter;

  capture_frequency       frequency;  
  guint32                 period;
  capture_mode            mode;
    
  gboolean                ftp_enable;
  gchar                  *ftp_address;
  gchar                  *ftp_path;
  gchar                  *ftp_user;
  gchar                  *ftp_passwd;
  
} capture_info;
/*
void
convert_to_yuv( dc1394_cameracapture *capture, unsigned char *src, unsigned char *dest, int mode);

void
convert_to_rgb( dc1394_cameracapture *capture, unsigned char *src, unsigned char *dest, int mode);
*/
gint
IsoStartThread(void);

void*
IsoReceiveThreadCleanup(void* arg);

void*
IsoReceiveThread(void* arg);

gint
IsoStopThread(void);

gint
PortholeStartThread(void);

void*
DisplayThreadCleanup(void* arg);

void*
PortholeDisplayThread(void* arg);

gint
PortholeStopThread(void);

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

gint
xvInit(void);

void
xvPut(void);

#endif

void
gdkPut(void);

gboolean
capture_single_frame(void);

gboolean
capture_multi_start(gchar *filename);

void
capture_multi_stop(void);

gint
capture_idler(gpointer p);

void
save_single_frame(gchar *filename);

gboolean
ftp_single_frame(gchar *filename, gchar *host, gchar *path, gchar *dest_filename, gchar *user, gchar *passwd);

#endif
