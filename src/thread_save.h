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

#include <gnome.h>

#ifndef __THREAD_SAVE_H__
#define __THREAD_SAVE_H__


typedef enum
{
  SAVE_SCRATCH_SEQUENTIAL,
  SAVE_SCRATCH_OVERWRITE
} save_scratch_t;

typedef enum
{
  SAVE_MODE_IMMEDIATE,
  SAVE_MODE_PERIODIC
} save_mode_t;


typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel_save;
  int                cancel_save_req;
  char               filename[256];
  char               filename_ext[256];
  long int           counter;
  unsigned char*     save_buffer;
  int                save_scratch;

} savethread_info_t;

gint
SaveStartThread(void);

void*
SaveCleanupThread(void* arg);

void*
SaveThread(void* arg);

gint
SaveStopThread(void);

#endif
