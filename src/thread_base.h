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

#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__

#include <pthread.h>
#include "thread_base.h"
#include "conversions.h"
#include <libdc1394/dc1394_control.h>
#include <time.h>

#define GUID_YUV12_PLANAR 0x32315659 
#define GUID_YUY2_PACKED  0x32595559
#define GUID_UYVY_PACKED  0x59565955
#define THREAD_LOOP_SLEEP_TIME_US 1

typedef enum _Service_T
{
  SERVICE_ISO=0,
  SERVICE_DISPLAY,
  SERVICE_SAVE,
  SERVICE_FTP,
  SERVICE_REAL
} service_t;

//typedef struct tm tm_t;

typedef struct _Buffer_T
{
  unsigned char *image;
  int width;
  int height;
  long int bytes_per_frame;
  int mode;
  int format;
  int bpp;
  bayer_decoding_t bayer;
  bayer_pattern_t bayer_pattern;
  stereo_decoding_t stereo_decoding;
  int format7_color_mode;

  // information about the time the frame was captured by the ISO thread.
  struct tm captime;
  int captime_millisec;
  char captime_string[19];

} buffer_t;

typedef struct _Chain_T
{
  pthread_mutex_t mutex_struct; // below is protected by mutex_struct
  struct _Chain_T* next_chain;
  struct _Chain_T* prev_chain;
  int             updated;
  service_t       service;

  pthread_mutex_t mutex_data; // below is protected by mutex_data
  pthread_t       thread;
  buffer_t*       next_buffer;
  buffer_t*       current_buffer;
  void*           data;
  buffer_t        local_param_copy; // not pointer: it remains in the chain.
} chain_t;


typedef enum _Clean_Mode_T
{
  CLEAN_MODE_NO_UI_UPDATE=0,
  CLEAN_MODE_UI_UPDATE,
  CLEAN_MODE_UI_UPDATE_NOT_ISO
} clean_mode_t;

chain_t*
GetService(service_t service, unsigned int camera);

void
CommonChainSetup(chain_t* chain, service_t req_service, unsigned int camera);

int
RollBuffers(chain_t* chain);

void
InsertChain(chain_t* chain, unsigned int camera);

void
RemoveChain(chain_t* chain, unsigned int camera);

void
InitChain(chain_t *chain, service_t service);

void
FreeChain(chain_t* chain);

void
convert_to_rgb(buffer_t *buffer, unsigned char *dest);

void
CleanThreads(clean_mode_t mode);

void
InitBuffer(buffer_t *buffer);

#endif
