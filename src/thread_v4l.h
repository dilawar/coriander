/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Dan Dennedy <dan@dennedy.org>
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

#ifndef __THREAD_V4L_H__
#define __THREAD_V4L_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/times.h>
#include <fcntl.h>
#include <math.h>
#include "videodev.h"
#include "definitions.h"
#include "support.h"
#include "thread_base.h"
#include "preferences.h"
#include "tools.h"

typedef struct
{
  //long int                period;
  pthread_mutex_t         mutex_cancel;
  int                     cancel_req;

  int v4l_dev;
  unsigned char *v4l_buffer;
  char v4l_dev_name[STRING_SIZE];

  int period;
  //int counter;

  struct video_capability vid_caps;
  struct video_window vid_win;
  struct video_picture vid_pic;

} v4lthread_info_t;


int
V4lShowFPS(gpointer *data);

gint
V4lStartThread(camera_t* cam);

void*
V4lCleanupThread(void* arg);

void*
V4lThread(void* arg);

gint
V4lStopThread(camera_t* cam);

void
V4lThreadCheckParams(chain_t *v4l_service);

void
swap_rb(unsigned char *image, int size);

#endif
