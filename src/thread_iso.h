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

#ifndef __THREAD_ISO_H__
#define __THREAD_ISO_H__

#define DMA_BUFFERS 10

typedef enum
{
  RECEIVE_METHOD_AUTO=0,
  RECEIVE_METHOD_RAW1394,
  RECEIVE_METHOD_VIDEO1394
} receive_method_t;


typedef struct
{ 
  raw1394handle_t         handle;
  receive_method_t        receive_method;
  dc1394_cameracapture    capture;

} isothread_info;

gint
IsoStartThread(void);

void*
IsoCleanupThread(void* arg);

void*
IsoThread(void* arg);

gint
IsoStopThread(void);

#endif
