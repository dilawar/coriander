/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_SDLLIB
#include "SDL.h"
#include "SDLEvent.h"
#endif

#include <math.h>
#include "support.h"
#include "definitions.h"
#include "thread_base.h"
#include "watch_thread.h"
#include "preferences.h"

typedef enum
{
  DISPLAY_REDRAW_OFF,
  DISPLAY_REDRAW_ON
} display_redraw_t;

typedef struct
{
  long int                period;
  pthread_mutex_t         mutex_cancel;
  int                     cancel_req;

  struct tms redraw_tms_buf;
  clock_t redraw_prev_time;
  clock_t redraw_current_time;

#ifdef HAVE_SDLLIB

  // general SDL stuff
  long unsigned int sdlflags;
  int sdlbpp;
  SDL_Surface *sdlvideo;   // video surface
  SDL_Overlay *sdloverlay; // video overlay surface
  SDL_Rect sdlvideorect;   // video rectangle for overlay surface

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
DisplayStartThread(camera_t* cam);

void*
DisplayCleanupThread(void* arg);

void*
DisplayThread(void* arg);

gint
DisplayStopThread(camera_t* cam);

void
ConditionalTimeoutRedraw(chain_t* service);

#ifdef HAVE_SDLLIB

int
SDLInit(chain_t *display_service);

void
convert_to_yuv_for_SDL(buffer_t *buffer, SDL_Overlay *sdloverlay);

void
SDLDisplayArea(chain_t *display_service);

void
SDLQuit(chain_t *display_service);

void
DisplayThreadCheckParams(chain_t *display_service);

#endif

#endif
