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

#ifndef __THREAD_SAVE_H__
#define __THREAD_SAVE_H__

#include <pthread.h>
#include <sys/times.h>
#include <math.h>
#include "support.h"
#include "thread_base.h"
#include "preferences.h"
#include "definitions.h"
#include "tools.h"

typedef enum
{
  SAVE_SCRATCH_SEQUENTIAL,
  SAVE_SCRATCH_OVERWRITE,
  SAVE_SCRATCH_SEQUENCE
} save_scratch_t;


typedef enum
{
  SAVE_CONVERT_ON,
  SAVE_CONVERT_OFF
} save_convert_t;


typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel_save;
  int                cancel_save_req;
  char               filename[STRING_SIZE];
  char               filename_ext[STRING_SIZE];
  long int           counter;
  unsigned char*     save_buffer;
  int                save_scratch;
  long int           period;
  int                rawdump;

  // timing data:
  struct tms tms_buf;
  clock_t prev_time;
  clock_t current_time;
  int frames;
  int timeout_func_id;

} savethread_info_t;

int
SaveShowFPS(gpointer *data);

gint
SaveStartThread(camera_t* cam);

void*
SaveCleanupThread(void* arg);

void*
SaveThread(void* arg);

gint
SaveStopThread(camera_t* cam);

void
Dump2File(char *name, chain_t *service);

void
SaveThreadCheckParams(chain_t *save_service);

#endif
