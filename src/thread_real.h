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


#ifndef __THREAD_REAL_H__
#define __THREAD_REAL_H__

#include <gnome.h>
#include "thread_base.h"
#include "definitions.h"
#include <time.h>
#include <sys/times.h>
 
#ifdef HAVE_REALLIB
#include "RealErrorSink.h"

typedef enum {
  REAL_COMPATIBILITY_0=0,
  REAL_COMPATIBILITY_1,
  REAL_COMPATIBILITY_2,
  REAL_COMPATIBILITY_3,
  REAL_COMPATIBILITY_4,
  REAL_COMPATIBILITY_5,
  REAL_COMPATIBILITY_6,
} RealCompatibility;

typedef enum {
  REAL_QUALITY_SMOOTH=0,
  REAL_QUALITY_NORMAL,
  REAL_QUALITY_SHARP,
  REAL_QUALITY_SLIDESHOW
} RealQuality;
#endif //HAVE_REALLIB

typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel_real;
  int                cancel_real_req;
  unsigned char*     real_buffer;
  char               realServerAddress[STRING_SIZE];
  char               realServerStreamName[STRING_SIZE];
  long unsigned int  realServerPort;
  char               realServerLogin[STRING_SIZE];
  char               realServerPassword[STRING_SIZE];
  char               streamTitle[STRING_SIZE];
  char               streamAuthor[STRING_SIZE];
  char               streamCopyright[STRING_SIZE];
  int                recordable;
  long int           audienceFlags;
  long int           videoQuality;
  int                realPlayerCompatibility;
  int                maxFrameRate;
  long int           period;

  // timing data:
  struct tms tms_buf;
  clock_t prev_time;
  clock_t current_time;
  int frames;
  int timeout_func_id;

#ifdef HAVE_REALLIB

  PN_RESULT res;
  // Remembers result of last function call.
  IRMABuildEngine *pBuildEngine;
  // main RealProducer control object
  IRMAInputPin *pVideoPin;
  // Input plug through which we will pass video samples (=frames) to the encoder.
  IRMAMediaSample *pSample;
  // Data structure used to pass frames to the video input pin.
  RealErrorSink *pErrorSinkObject;
  // Error Sink object

#endif //HAVE_REALLIB

} realthread_info_t;

#ifdef __cplusplus
extern "C" {
#endif

gint
RealStartThread(void);

void*
RealCleanupThread(void* arg);

int
RealShowFPS(gpointer *data);

void*
RealThread(void* arg);

gint
RealStopThread(void);

void
RealThreadCheckParams(chain_t *real_service);

int
RealSetup(realthread_info_t *info, chain_t *service);
  
#ifdef __cplusplus
}
#endif
#endif
