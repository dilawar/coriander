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

#ifndef __THREAD_FTP_H__
#define __THREAD_FTP_H__


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_FTPLIB
#include <ftplib.h>
#endif

#include <pthread.h>
#include <sys/times.h>
#include <gnome.h>
#include <math.h>
#include "support.h"
#include "definitions.h"
#include "thread_base.h"
#include "tools.h"


typedef enum
{
  FTP_SCRATCH_SEQUENTIAL=0,
  FTP_SCRATCH_OVERWRITE
} ftp_scratch_t;
 
typedef enum
{
  FTP_TAG_DATE=0,
  FTP_TAG_NUMBER
} ftp_tag_t;
 
typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel;
  int                cancel_req;
  char               filename[STRING_SIZE];
  char               filename_ext[STRING_SIZE];
  char               address[STRING_SIZE];
  char               password[STRING_SIZE];
  char               user[STRING_SIZE];
  char               path[STRING_SIZE];
  long int           period;
  long int           counter;
  long int           imlib_buffer_size;
  unsigned char*     buffer;
  int                scratch;
  int                datenum;
#ifdef HAVE_FTPLIB
  netbuf             *ftp_handle;
#endif

} ftpthread_info_t;

gint
FtpStartThread(camera_t* cam);

void*
FtpCleanupThread(void* arg);

void*
FtpThread(void* arg);

gint
FtpStopThread(camera_t* cam);

void
FtpThreadCheckParams(chain_t *ftp_service);

#ifdef HAVE_FTPLIB

gboolean
OpenFtpConnection(ftpthread_info_t* info);

gboolean
FtpPutFrame(char *filename, GdkImlibImage *im, ftpthread_info_t* info);

void
CloseFtpConnection(netbuf *ftp_handle);

gboolean
CheckFtpConnection(ftpthread_info_t* info);

#endif

#endif
