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

#ifndef __THREAD_ISO_H__
#define __THREAD_ISO_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <libdc1394/dc1394_control.h>
#include <math.h>
#include "support.h"
#include "thread_base.h"
#include "topology.h"
#include "definitions.h"
#include "tools.h" 

#define DMA_BUFFERS 4

typedef enum
{
  RECEIVE_METHOD_RAW1394=0, 
  RECEIVE_METHOD_VIDEO1394
} receive_method_t;

typedef struct
{ 
  raw1394handle_t         handle;
  receive_method_t        receive_method;
  dc1394_cameracapture    capture;
  int video1394_dropframes;
  char video1394_device[STRING_SIZE];
  // timing data:
  struct tms tms_buf;
  clock_t prev_time;
  clock_t current_time;
  int frames;
  int timeout_func_id;

  unsigned char *temp;
  int temp_size;
  int temp_allocated;

  int orig_sizex;
  int orig_sizey;
  int cond16bit;

  struct timeb rawtime;

} isothread_info_t;

int
IsoShowFPS(gpointer *data);

gint
IsoStartThread(camera_t* cam);

void*
IsoCleanupThread(void* arg);

void*
IsoThread(void* arg);

gint
IsoStopThread(camera_t* camera);

void
IsoThreadCheckParams(chain_t *iso_service);

void
SetColorMode(buffer_t *buffer);

#endif
