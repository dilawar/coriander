/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *
 * PVN saving capability by Jacob (Jack) Gryn and Konstantinos G. Derpanis
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

typedef enum
{
  SAVE_FORMAT_PNG=0,
  SAVE_FORMAT_JPEG,
  SAVE_FORMAT_TIFF,
  SAVE_FORMAT_PPMPGM,
  SAVE_FORMAT_XPM,
  SAVE_FORMAT_EIM,
  SAVE_FORMAT_RAW,
  SAVE_FORMAT_MPEG,
  SAVE_FORMAT_PVN,
  SAVE_FORMAT_RAW_VIDEO
} save_format_t;

typedef enum
{
  SAVE_APPEND_NONE=0,
  SAVE_APPEND_DATE_TIME,
  SAVE_APPEND_NUMBER
} save_append_t;

typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread. */

  pthread_mutex_t    mutex_cancel;
  int                cancel_req;
  char               filename_base[STRING_SIZE];
  char               filename_ext[STRING_SIZE];
  char               destdir[STRING_SIZE];
  unsigned char*     buffer;
  save_format_t      format;
  save_append_t      append;
  long int           period;
  int                rawdump;
  int                save_to_dir;
  unsigned char*     bigbuffer;
  unsigned long int  bigbuffer_position;
  unsigned long int  ram_buffer_size;
  int                use_ram_buffer;

  //MPEG encoding data
  AVOutputFormat *fmt;
  AVFormatContext *oc;
  AVStream *video_st;
  AVFrame *picture;
  AVFrame *tmp_picture_yuv411;
  AVFrame *tmp_picture_yuv422;
  
  char subtitle[256];
  int fdts;

} savethread_info_t;

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

int
isColor(int buffer_color_mode);

int
needsConversionForPVN(int buffer_color_mode);

int
getConvertedBytesPerChannel(int buffer_color_mode);

int
getBytesPerChannel(int buffer_color_mode);

double
framerateAsDouble(int framerate_enum);

float
getAvgBytesPerPixel(int buffer_color_mode);

unsigned int
getDepth(unsigned long bufsize, int mode, unsigned int height, unsigned int width);

void
convert_for_pvn(unsigned char *buffer, unsigned int width, unsigned int height,
		unsigned int page, int buffer_color_mode, unsigned char *dest);

void
InitVideoFile(chain_t *save_service, FILE *fd, char *filename_out);

void
writePVNHeader(FILE *fd, unsigned int mode, unsigned int height, unsigned int width,
	       unsigned int depth, unsigned int bpp, double framerate);

int
GetSaveFD(chain_t *save_service, FILE **fd, char *filename_out);

void
FillRamBuffer(chain_t *save_service);


#endif // __THREAD_SAVE_H__
