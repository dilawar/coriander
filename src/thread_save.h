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
  SAVE_SCRATCH_SEQUENTIAL=0,
  SAVE_SCRATCH_OVERWRITE,
  SAVE_SCRATCH_VIDEO
} save_scratch_t;

typedef enum
{
  SAVE_TAG_DATE=0,
  SAVE_TAG_NUMBER
} save_tag_t;
 

typedef enum
{
  SAVE_CONVERT_ON=0,
  SAVE_CONVERT_OFF
} save_convert_t;


typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel;
  int                cancel_req;
  char               filename[STRING_SIZE];
  char               filename_ext[STRING_SIZE];
  long int           counter;
  unsigned char*     buffer;
  int                scratch;
  int                datenum;
  long int           period;
  int                rawdump;
  unsigned char*     bigbuffer;
  unsigned long int  bigbuffer_position;
  unsigned long int  ram_buffer_size;
  int                use_ram_buffer;

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
SaveThreadCheckParams(chain_t *save_service);

#endif
