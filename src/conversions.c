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

/*macro used to convert a YUV pixel to RGB format
  from Bart Nabbe

  v*1434 / 2048
  (u*406 - v*595)/2048
  u*2078 / 2048
  bitshift performance enhanced by Damien Douxchamps
*/

#define YUV2RGB(y, u, v, r, g, b)\
  r = y + ( ((v << 10) + (v << 8) + (v << 7) + (v << 4) + (v << 3) + (v << 1)) >> 11);\
  g = y - ( ((u << 8) + (u << 7) + (u << 4) + (u << 2) + (u << 1)) >> 11 ) - \
          ( ((v << 9) + (v << 6) + (v << 4) + (v << 1) + v) >> 11 );\
  b = y + ( ((u << 11) + (u << 5) - (u << 1)) >> 11);\
  r = r < 0 ? 0 : r;\
  g = g < 0 ? 0 : g;\
  b = b < 0 ? 0 : b;\
  r = r > 255 ? 255 : r;\
  g = g > 255 ? 255 : g;\
  b = b > 255 ? 255 : b
  

#define RGB2YUV(r, g, b, y, u, v)\
  y = (9798*r + 19235*g + 3736*b)  >> 15;\
  u = ((-4784*r - 9437*g + 14221*b) >> 15)  + 128;\
  v = ((20218*r - 16941*g - 3277*b) >> 15) + 128;\
  y = y < 0 ? 0 : y;\
  u = u < 0 ? 0 : u;\
  v = v < 0 ? 0 : v;\
  y = y > 255 ? 255 : y;\
  u = u > 255 ? 255 : u;\
  v = v > 255 ? 255 : v

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS BETWEEN       UYVY    AND    YUYV
 *
 **********************************************************************/

inline void
uyvy2yuyv (unsigned char *src, unsigned char *dest, int NumPixels) {
	swab(src, dest, NumPixels << 1);
}

inline void
yuyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
	swab(src, dest, NumPixels << 1);
}

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/


inline void
uyyvyy2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i=NumPixels + (NumPixels >> 1);
  register int j=NumPixels << 1;
  register int y0, y1, y2, y3, u, v;

  while (i > 0)
    {
      y3 = src[i--];
      y2 = src[i--];
      v  = src[i--];
      y1 = src[i--];
      y0 = src[i--];
      u  = src[i--];

      dest[j--] = y3;
      dest[j--] = v;
      dest[j--] = y2;
      dest[j--] = u;

      dest[j--] = y1;
      dest[j--] = v;
      dest[j--] = y0;
      dest[j--] = u;

    }
}

inline void
uyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + (NumPixels << 1);
  register int j = NumPixels << 1;
  register int y0, y1, u0, u1, v0, v1;

  while (i > 0)
    {
      v1 = src[i--];
      y1 = src[i--];
      u1 = src[i--];
      v0 = src[i--];
      y0 = src[i--];
      u0 = src[i--];

      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
    }
}

inline void
y2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i= NumPixels;
  register int j = NumPixels << 1;
  register int y0, y1;

  while (i > 0)
    {
      y1 = src[i--];
      y0 = src[i--];

      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
    }
}

inline void
y162uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels << 1;
  register int j = NumPixels << 1;
  register int y0, y1;

  while (i > 0)
    {
      i--;
      y1   = src[i--];
      i--;
      y0   = src[i--];

      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
    }
}

inline void
rgb2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + ( NumPixels << 1 );
  register int j = NumPixels << 1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  while (i > 0)
    {
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);

      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
    }
}

inline void
rgb482uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = (NumPixels + ( NumPixels << 1 )) << 1;
  register int j = NumPixels << 1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  while (i > 0)
    {
      i--;
      b = (unsigned char) src[i--];
      i--;
      g = (unsigned char) src[i--];
      i--;
      r = (unsigned char) src[i--];
      i--;
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (unsigned char) src[i--];
      i--;
      g = (unsigned char) src[i--];
      i--;
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);

      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
    }
}

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO RGB 24bpp 
 *
 **********************************************************************/

inline void
rgb482rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = (NumPixels + ( NumPixels << 1 )) << 1;
  register int j = NumPixels + ( NumPixels << 1 );

  while (i > 0)
    {
      i--;
      dest[j--]=src[i--];
      i--;
      dest[j--]=src[i--];
      i--;
      dest[j--]=src[i--];
    }
}


inline void
uyv2rgb (unsigned char *src, unsigned char *dest, int NumPixels)
{
  register int i = NumPixels << 1;
  register int j = NumPixels + ( NumPixels << 1 );
  register int y, u, v;
  register int r, g, b;

  while (i > 0)
    {
      v = (unsigned char) src[i--] - 128;
      y = (unsigned char) src[i--];
      u = (unsigned char) src[i--] - 128;
      YUV2RGB (y, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
      
    }
}

inline void
uyvy2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels << 1;
  register int j = NumPixels + ( NumPixels << 1 );
  register int y0, y1, u, v;
  register int r, g, b;

  while (i > 0)
    {
      y1 = (unsigned char) src[i--];
      v  = (unsigned char) src[i--] - 128;
      y0 = (unsigned char) src[i--];
      u  = (unsigned char) src[i--] - 128;
      YUV2RGB (y1, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
      YUV2RGB (y0, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
    }
}


inline void
uyyvyy2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + ( NumPixels >> 1 );
  register int j = NumPixels + ( NumPixels << 1 );
  register int y0, y1, y2, y3, u, v;
  register int r, g, b;
  
  while (i > 0)
    {
      y3 = (unsigned char) src[i--];
      y2 = (unsigned char) src[i--];
      v  = (unsigned char) src[i--] - 128;
      y1 = (unsigned char) src[i--];
      y0 = (unsigned char) src[i--];
      u  = (unsigned char) src[i--] - 128;
      YUV2RGB (y3, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
      YUV2RGB (y2, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
      YUV2RGB (y1, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
      YUV2RGB (y0, u, v, r, g, b);
      dest[j--] = b;
      dest[j--] = g;
      dest[j--] = r;
    }
}

inline void
y2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels;
  register int j = NumPixels + ( NumPixels << 1 );
  register int y;

  while (i > 0)
    {
      y = (unsigned char) src[i--];
      dest[j--] = y;
      dest[j--] = y;
      dest[j--] = y;
    }
}

inline void
y162rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels << 1;
  register int j = NumPixels + ( NumPixels << 1 );
  register int y;

  while (i > 0)
    {
      i--;
      y = (unsigned char)src[i--];
      dest[j--] = y;
      dest[j--] = y;
      dest[j--] = y;
    }
}

