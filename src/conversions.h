/*
 * Copyright (C) 2000-2001 Dan Dennedy  <dan@dennedy.org>
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

inline void
uyvy2yuyv (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
yuyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyyvyy2yuyv (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
yuv2yuyv (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
y2yuyv (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
rgb2yuyv (unsigned char *RGB, unsigned char *YUV, int NumPixels);

inline void
yuv2rgb (unsigned char *YUV, unsigned char *RGB, int NumPixels);

inline void
uyvy2rgb (unsigned char *YUV, unsigned char *RGB, int NumPixels);

inline void
yuyv2rgb (unsigned char *YUV, unsigned char *RGB, int NumPixels);

inline void
uyyvyy2rgb (unsigned char *YUV, unsigned char *RGB, int NumPixels);

inline void
y2rgb (unsigned char *YUV, unsigned char *RGB, int NumPixels);
/*
 * Copyright (C) 2000-2001 Dan Dennedy  <dan@dennedy.org>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include "conversions.h"

extern void swab();

inline void
uyvy2yuyv (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
yuyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyyvyy2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
y2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
y162uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
rgb2uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
rgb482uyvy (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
rgb482rgb (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyv2rgb (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyvy2rgb (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
uyyvyy2rgb (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
y2rgb (unsigned char *src, unsigned char *dest, int NumPixels);

inline void
y162rgb (unsigned char *src, unsigned char *dest, int NumPixels);


#endif
