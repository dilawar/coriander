/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
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

#ifndef __THREAD_DISPLAY_H__
#define __THREAD_DISPLAY_H__

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>

#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "thread_base.h" 


typedef enum
{
  DISPLAY_METHOD_AUTO=0,
  DISPLAY_METHOD_XV,
  DISPLAY_METHOD_GDK
} display_method_t;

typedef struct
{
  gboolean                is_open;
  GtkWidget              *drawable;
  display_method_t        display_method;
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
  Display                *display;
  Window                  window;
  unsigned long           xv_format;
  XvImage                *xv_image;
  int                     xv_port;
  XShmSegmentInfo         xv_shm_info;
  GC                      xv_gc;
#endif
  long int                period;
  char                   *gdk_buffer;
  pthread_mutex_t         mutex_cancel_display;
  int                     cancel_display_req;
} displaythread_info_t;

gint
DisplayStartThread(void);

void*
DisplayCleanupThread(void* arg);

void*
DisplayThread(void* arg);

gint
DisplayStopThread(unsigned int camera);

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

gint
xvInit(chain_t *display_service);

void
xvPut(chain_t *display_service);

void
convert_to_yuv_for_xv(unsigned char *src, unsigned char *dest, int mode, int width, int height, long int bytes_per_frame, int xv_format);

#endif

void
gdkPut(chain_t *display_service);


#endif
