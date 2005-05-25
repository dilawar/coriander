/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Dan Dennedy  <dan@dennedy.org>
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

#include "coriander.h"

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/

void
convert_to_rgb(buffer_t *buffer, unsigned char *dest)
{
  switch(buffer->color_mode) {
  case DC1394_COLOR_CODING_MONO8:
  case DC1394_COLOR_CODING_RAW8:
    dc1394_MONO8_to_RGB8(buffer->image,dest,buffer->width*buffer->height);
    break;
  case DC1394_COLOR_CODING_YUV411:
    dc1394_YUV411_to_RGB8(buffer->image,dest,buffer->width*buffer->height);
    break;
  case DC1394_COLOR_CODING_YUV422:
    dc1394_YUV422_to_RGB8(buffer->image,dest,buffer->width*buffer->height);
    break;
  case DC1394_COLOR_CODING_YUV444:
    dc1394_YUV444_to_RGB8(buffer->image,dest,buffer->width*buffer->height);
    break;
  case DC1394_COLOR_CODING_RGB8:
    memcpy(dest,buffer->image,3*buffer->width*buffer->height);
    break;
  case DC1394_COLOR_CODING_MONO16:
  case DC1394_COLOR_CODING_RAW16:
    dc1394_MONO16_to_RGB8(buffer->image,dest,buffer->width*buffer->height,buffer->bpp);
    break;
  case DC1394_COLOR_CODING_RGB16:
    dc1394_RGB16_to_RGB8(buffer->image,dest,buffer->width*buffer->height);
    break;
  }
}

// we should optimize this for RGB too: RGB modes could use RGB-SDL instead of YUV overlay
void
convert_to_yuv_for_SDL(buffer_t *buffer, SDL_Overlay *sdloverlay, int overlay_byte_order)
{
  unsigned char *dest=sdloverlay->pixels[0];
  /*
  fprintf(stderr,"C:[%d %d] BPF:%lli ColMode:%d\n",
	  buffer->width, buffer->height,
	  buffer->bytes_per_frame,
	  buffer->color_mode);
  */
  switch(buffer->color_mode) {
  case DC1394_COLOR_CODING_MONO8:
  case DC1394_COLOR_CODING_RAW8:
    dc1394_MONO8_to_YUV422(buffer->image, dest, buffer->width, buffer->height, 
			   sdloverlay->pitches[0], overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_YUV411:
    dc1394_YUV411_to_YUV422(buffer->image,dest,buffer->width*buffer->height, overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_YUV422:
    dc1394_YUV422_to_YUV422(buffer->image,dest,buffer->width*buffer->height, overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_YUV444:
    dc1394_YUV444_to_YUV422(buffer->image,dest,buffer->width*buffer->height, overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_RGB8:
    dc1394_RGB8_to_YUV422(buffer->image,dest,buffer->width*buffer->height, overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_MONO16:
  case DC1394_COLOR_CODING_RAW16:
    dc1394_MONO16_to_YUV422(buffer->image,dest,buffer->width*buffer->height,buffer->bpp, overlay_byte_order);
    break;
  case DC1394_COLOR_CODING_RGB16:
    dc1394_RGB16_to_YUV422(buffer->image,dest,buffer->width*buffer->height, overlay_byte_order);
    break;
  }
}

void
convert_for_pvn(unsigned char *buffer, unsigned int width, unsigned int height,
		unsigned int page, int color_mode, unsigned char *dest)
{
  float bpp;
  unsigned char *buf_loc;

  dc1394_get_bytes_per_pixel(color_mode, &bpp);
  buf_loc=buffer+(int)(page*bpp*width*height);
  if(dest==NULL)
    return;

  switch(color_mode) {
    case DC1394_COLOR_CODING_MONO8:
    case DC1394_COLOR_CODING_RAW8:
      memcpy(dest,buf_loc,width*height);
      break;
    case DC1394_COLOR_CODING_YUV411:
      dc1394_YUV411_to_RGB8(buf_loc,dest,width*height);
      break;
    case DC1394_COLOR_CODING_YUV422:
      dc1394_YUV422_to_RGB8(buf_loc,dest,width*height);
      break;
    case DC1394_COLOR_CODING_YUV444:
      dc1394_YUV444_to_RGB8(buf_loc,dest,width*height);
      break;
    case DC1394_COLOR_CODING_RGB8:
      memcpy(dest,buf_loc,3*width*height);
      break;
    case DC1394_COLOR_CODING_MONO16:
    case DC1394_COLOR_CODING_MONO16S:
    case DC1394_COLOR_CODING_RAW16:
      memcpy(dest,buf_loc,2*width*height);
      break;
    case DC1394_COLOR_CODING_RGB16:
      memcpy(dest,buf_loc,6*width*height);
      break;
    default:
      fprintf(stderr, "Unknown buffer format!\n");
      break;
  }
}
