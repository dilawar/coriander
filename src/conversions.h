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

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

typedef enum
{
  NO_BAYER_DECODING=0,
  BAYER_DECODING_NEAREST,
  BAYER_DECODING_EDGE_SENSE,
  BAYER_DECODING_DOWNSAMPLE,
  BAYER_DECODING_SIMPLE
} bayer_decoding_t;

typedef enum
{
  NO_STEREO_DECODING=0,
  STEREO_DECODING_INTERLACED,
  STEREO_DECODING_FIELD
} stereo_decoding_t;

enum
{
  OVERLAY_BYTE_ORDER_YUYV=0,
  OVERLAY_BYTE_ORDER_UYVY
};

// UYVY <-> YUYV
void
uyvy2yuyv (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

void
yuyv2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

// XXX -> UYVY
void
uyyvyy2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

void
uyv2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

void
y2uyvy (unsigned char *src, unsigned char *dest, 
	unsigned long src_width, unsigned long src_height,
	unsigned long dest_pitch, int byte_order);

void
y162uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits, int byte_order);

void
y162y (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits);

void
rgb2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

void
rgb482uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order);

// XXX -> RGB
void
rgb482rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

void
uyv2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

void
uyvy2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

void
uyyvyy2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

void
y2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

void
y162rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits);

// BAYER -> RGB
void
BayerNearestNeighbor(unsigned char *src, unsigned char *dest, int sx, int sy, int type);

void
BayerEdgeSense(unsigned char *src, unsigned char *dest, int sx, int sy, int type);

void
BayerDownsample(unsigned char *src, unsigned char *dest, int sx, int sy, int type);

void
BayerSimple(unsigned char *src, unsigned char *dest, int sx, int sy, int type);

void
StereoDecode(unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

#endif // __CONVERSIONS_H__
