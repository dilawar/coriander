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
  NO_STEREO_DECODING=0,
  STEREO_DECODING_INTERLACED,
  STEREO_DECODING_FIELD
} stereo_decoding_t;

enum
{
  OVERLAY_BYTE_ORDER_YUYV=0,
  OVERLAY_BYTE_ORDER_UYVY
};

// color conversion functions from Bart Nabbe.
// corrected by Damien: bad coeficients in YUV2RGB
#define YUV2RGB(y, u, v, r, g, b)\
  r = y + ((v*1436) >> 10);\
  g = y - ((u*352 + v*731) >> 10);\
  b = y + ((u*1814) >> 10);\
  r = r < 0 ? 0 : r;\
  g = g < 0 ? 0 : g;\
  b = b < 0 ? 0 : b;\
  r = r > 255 ? 255 : r;\
  g = g > 255 ? 255 : g;\
  b = b > 255 ? 255 : b
  

#define RGB2YUV(r, g, b, y, u, v)\
  y = (306*r + 601*g + 117*b)  >> 10;\
  u = ((-172*r - 340*g + 512*b) >> 10)  + 128;\
  v = ((512*r - 429*g - 83*b) >> 10) + 128;\
  y = y < 0 ? 0 : y;\
  u = u < 0 ? 0 : u;\
  v = v < 0 ? 0 : v;\
  y = y > 255 ? 255 : y;\
  u = u > 255 ? 255 : u;\
  v = v > 255 ? 255 : v

#define REPLPIX(im, pix, index)\
  im[index]=pix[0];\
  im[index+1]=pix[1];\
  im[index+2]=pix[2];\
  im[index+3]=pix[3]

#define INVPIX(im, index)\
  im[index]=255-im[index];\
  im[index+1]=255-im[index+1];\
  im[index+2]=255-im[index+2];\
  im[index+3]=255-im[index+3]


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

/*

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
ClearBorders(unsigned char* dest, int sx, int sy, int w);

*/

void
StereoDecode(unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);

#endif // __CONVERSIONS_H__
