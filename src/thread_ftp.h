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

#ifndef __THREAD_FTP_H__
#define __THREAD_FTP_H__

#ifdef HAVE_FTPLIB
#include <ftplib.h>
#endif 

#include <pthread.h>
typedef enum
{
  FTP_SCRATCH_SEQUENTIAL,
  FTP_SCRATCH_OVERWRITE
} ftp_scratch_t;

typedef enum
{
  FTP_MODE_IMMEDIATE,
  FTP_MODE_PERIODIC
} ftp_mode_t;


typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel_ftp;
  int                cancel_ftp_req;
  char               filename[256];
  char               filename_ext[256];
  char               address[256];
  char               password[256];
  char               user[256];
  char               path[256];
  long int           counter;
  long int           imlib_buffer_size;
  unsigned char*     ftp_buffer;
  int                ftp_scratch;
#ifdef HAVE_FTPLIB
  netbuf             *ftp_handle;
#endif

} ftpthread_info_t;

gint
FtpStartThread(void);

void*
FtpCleanupThread(void* arg);

void*
FtpThread(void* arg);

gint
FtpStopThread(void);

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
