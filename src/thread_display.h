/*
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
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

#ifdef HAVE_SDLLIB
#include "SDL.h"
#endif

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "thread_base.h" 
#include <time.h>
#include <sys/times.h>

typedef struct
{
  long int                period;
  pthread_mutex_t         mutex_cancel_display;
  int                     cancel_display_req;

  // timing data:
  struct tms tms_buf;
  clock_t prev_time;
  clock_t current_time;
  int frames;
  int timeout_func_id;

#ifdef HAVE_SDLLIB

  // general SDL stuff
  long unsigned int SDL_flags;
  int SDL_bpp;
  SDL_Surface *SDL_video;   // video surface
  SDL_Overlay *SDL_overlay; // video overlay surface
  SDL_Rect SDL_videoRect;   // video rectangle for overlay surface

  // events
  pthread_mutex_t         mutex_cancel_event;
  int                     cancel_event_req;
  pthread_mutex_t         mutex_event;
  pthread_t               event_thread;

#endif

} displaythread_info_t;

int
DisplayShowFPS(gpointer *data);

gint
DisplayStartThread(void);

void*
DisplayCleanupThread(void* arg);

void*
DisplayThread(void* arg);

gint
DisplayStopThread(unsigned int camera);

#ifdef HAVE_SDLLIB

int
sdlInit(chain_t *display_service);

void
convert_to_yuv_for_SDL(unsigned char *src, unsigned char *dest, int mode,
		       int width, int height, int f7_colormode, int bayer, int bits);

void
SDLDisplayArea(chain_t *display_service);

#endif

#endif
