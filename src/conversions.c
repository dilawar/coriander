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

#include "conversions.h"

extern void swab();

// The following #define is there for the users who experience green/purple
// images in the display. This seems to be a videocard driver problem.

#define YUYV // instead of the standard UYVY

// color conversion functions from Bart Nabbe.
// corrected by Damien: bad coeficients in YUV2RGB
#define YUV2RGB(y, u, v, r, g, b)\
  r = y + ((v*1436) >>10);\
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

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/

void
yuyv2uyvy(unsigned char *src, unsigned char *dest, int NumPixels) {
#ifdef YUYV
  swab(src, dest, NumPixels << 1);
#else
  memcpy(dest,src, NumPixels<<1);
#endif
}

void
uyvy2yuyv(unsigned char *src, unsigned char *dest, int NumPixels) {
#ifdef YUYV
  swab(src, dest, NumPixels << 1);
#else
  memcpy(dest,src, NumPixels<<1);
#endif
}
void
uyyvyy2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i=NumPixels + (NumPixels >> 1)-1;
  register int j=(NumPixels << 1)-1;
  register int y0, y1, y2, y3, u, v;

  while (i > 0)
    {
      y3 = src[i--];
      y2 = src[i--];
      v  = src[i--];
      y1 = src[i--];
      y0 = src[i--];
      u  = src[i--];
#ifdef YUYV
      dest[j--] = v;
      dest[j--] = y3;
      dest[j--] = u;
      dest[j--] = y2;

      dest[j--] = v;
      dest[j--] = y1;
      dest[j--] = u;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y3;
      dest[j--] = v;
      dest[j--] = y2;
      dest[j--] = u;

      dest[j--] = y1;
      dest[j--] = v;
      dest[j--] = y0;
      dest[j--] = u;
#endif
    }
}

void
uyv2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1;

  while (i > 0)
    {
      v1 = src[i--];
      y1 = src[i--];
      u1 = src[i--];
      v0 = src[i--];
      y0 = src[i--];
      u0 = src[i--];

#ifdef YUYV
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
#endif
    }
}


void
y2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i= NumPixels-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1;

  while (i > 0)
    {
      y1 = src[i--];
      y0 = src[i--];
#ifdef YUYV
      dest[j--] = 128;
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
#endif
    }
}

void
y162uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1;

  while (i > 0)
    {
      i--;
      y1   = src[i--];
      i--;
      y0   = src[i--];
#ifdef YUYV
      dest[j--] = 128;
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
#endif
    }
}

void
rgb2uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + ( NumPixels << 1 )-1;
  register int j = (NumPixels << 1)-1;
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
#ifdef YUYV
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
#endif
    }
}

void
rgb482uyvy (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = ( (NumPixels + ( NumPixels << 1 )) << 1 ) -1;
  register int j = (NumPixels << 1)-1;
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

#ifdef YUYV
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y0;
#else // UYVY
      dest[j--] = y1;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
#endif
    }
}

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO RGB 24bpp 
 *
 **********************************************************************/

void
rgb482rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = ((NumPixels + ( NumPixels << 1 )) << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;

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


void
uyv2rgb (unsigned char *src, unsigned char *dest, int NumPixels)
{
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
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

void
uyvy2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
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


void
uyyvyy2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels + ( NumPixels >> 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
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

void
y2rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = NumPixels-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i > 0)
    {
      y = (unsigned char) src[i--];
      dest[j--] = y;
      dest[j--] = y;
      dest[j--] = y;
    }
}

void
y162rgb (unsigned char *src, unsigned char *dest, int NumPixels) {
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
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

/****************************************************************
 *     Color conversion functions for cameras that can          * 
 * output raw-Bayer pattern images, such as some Basler cameras *
 * Most of the algos presented here com from                    *
 * http://ise0.Stanford.EDU/~tingchen/main.htm                  *
 * and have been converted from Matlab to C                     *
 ****************************************************************/

void
BayerNearestNeighbor(unsigned char *src, unsigned char *dest, int sx, int sy)
{
  unsigned char *outR, *outG, *outB;
  register int i,j;

  // sx and sy should be even
  // first pixel should be Green, second red:
  // G R G R G R G R
  // B G B G B G B G
  // G R G R G R G R
  outR=&dest[0];
  outG=&dest[sx*sy];
  outB=&dest[sx*sy*2];

  // R channel
  for (i=0;i<sy;i+=2)//every two lines
    { //get one full line
      for (j=0;j<sx;j+=2)
	{
	  outR[i*sx+j]=src[i*sx+j+1];
	  outR[i*sx+j+1]=outR[i*sx+j];
	}
      // copy it for the next line
      for (j=0;j<sx;j++)
	outR[(i+1)*sx+j]=outR[i*sx+j];
    }
      
  // B channel
  for (i=0;i<sy;i+=2)//every two lines
    { //get one full line
      for (j=0;j<sx;j+=2)
	{
	  outB[i*sx+j]=src[(i+1)*sx+j];
	  outB[i*sx+j+1]=outB[i*sx+j];
	}
      // copy it for the next line
      for (j=0;j<sx;j++)
	outB[(i+1)*sx+j]=outB[i*sx+j];
    }

  // G channel
  for (i=0;i<sy;i+=2)//every two lines
    { //get one full line
      for (j=0;j<sx;j+=2)
	{
	  outG[i*sx+j]=src[i*sx+j];
	  outG[i*sx+j+1]=outB[(i+1)*sx+(j+1)];
	}
      // copy it for the next line
      for (j=0;j<sx;j++)
	outG[(i+1)*sx+j]=outG[i*sx+j];
    }
}


void
BayerEdgeSense2(unsigned char *src, unsigned char *dest, int sx, int sy)
{
  unsigned char *outR, *outG, *outB;
  register int i,j;
  int dh, dv;

  // There is no boundary check (<0, >255). This should be implmented later.

  // sx and sy should be even
  // first pixel should be Green, second red:
  // G R G R G R G R
  // B G B G B G B G
  // G R G R G R G R

  outR=&dest[0];
  outG=&dest[sx*sy];
  outB=&dest[sx*sy*2];

  // copy original RGB data to output images
  for (i=0;i<sy;i+=2)
    for (j=0;j<sx;j+=2)
      {
	outG[i*sx+j]=src[i*sx+j];
	outG[(i+1)*sx+(j+1)]=src[(i+1)*sx+(j+1)];
	outR[i*sx+j+1]=src[i*sx+j+1];
	outB[(i+1)*sx+j]=src[(i+1)*sx+j];
      }

  // process GREEN channel
  for (i=3;i<sx-2;i+=2)
    for (j=2;j<sy-3;j+=2)
      {
	dh=abs((outB[(j-2)*sx+i]+outB[(j+2)*sx+i])/2-outB[j*sx+i]);
	dv=abs((outB[j*sx+i-2]+outB[j*sx+i+2])/2-outB[j*sx+i]);
	if (dh<dv)
	  outG[j*sx+i]=(outG[(j-1)*sx+i]+outG[(j+1)*sx+i])/2;
	else
	  if (dh>dv)
	    outG[j*sx+i]=(outG[j*sx+i-1]+outG[j*sx+i+1])/2;
	  else
	    outG[j*sx+i]=(outG[j*sx+i-1]+outG[j*sx+i+1]+outG[(j-1)*sx+i]+outG[(j+1)*sx+i])/4;
      }

  for (i=2;i<sx-3;i+=2)
    for (j=3;j<sy-2;j+=2)
      {
	dh=abs((outR[(j-2)*sx+i]+outR[(j+2)*sx+i])/2-outR[j*sx+i]);
	dv=abs((outR[j*sx+i-2]+outR[j*sx+i+2])/2-outR[j*sx+i]);
	if (dh<dv)
	  outG[j*sx+i]=(outG[(j-1)*sx+i]+outG[(j+1)*sx+i])/2;
	else
	  if (dh>dv)
	    outG[j*sx+i]=(outG[j*sx+i-1]+outG[j*sx+i+1])/2;
	  else
	    outG[j*sx+i]=(outG[j*sx+i-1]+outG[j*sx+i+1]+outG[(j-1)*sx+i]+outG[(j+1)*sx+i])/4;
      }

  // process RED channel
  for (i=0;i<sx-1;i+=2)
    for (j=2;j<sy-1;j+=2)
      outR[j*sx+i]=outG[j*sx+i]+(outR[(j-1)*sx+i]-outG[(j-1)*sx+i]+outR[(j+1)*sx+i]-outG[(j+1)*sx+i])/2;

  for (i=1;i<sx-2;i+=2)
    {
      for (j=1;j<sy;j+=2)
	outR[j*sx+i]=outG[j*sx+i]+(outR[j*sx+i-1]-outG[j*sx+i-1]+outR[j*sx+i+1]-outG[j*sx+i+1])/2;
      for (j=2;j<sy-1;j+=2)
	outR[j*sx+i]=(outR[(j-1)*sx+i-1]-outG[(j-1)*sx+i-1]+outR[(j+1)*sx+i-1]-outG[(j+1)*sx+i-1]+
		      outR[(j-1)*sx+i+1]-outG[(j-1)*sx+i+1]+outR[(j+1)*sx+i+1]-outG[(j+1)*sx+i+1])/4;
    }

  // process BLUE channel
  for (i=1;i<sx;i+=2)
    for (j=1;j<sy;j+=2)
      outB[j*sx+i]=outG[j*sx+i]+(outB[(j-1)*sx+i]-outG[(j-1)*sx+i]+outB[(j+1)*sx+i]-outG[(j+1)*sx+i])/2;

  for (i=2;i<sx-1;i+=2)
    {
      for (j=0;j<sy-1;j+=2)
	outB[j*sx+i]=outG[j*sx+i]+(outB[j*sx+i-1]-outG[j*sx+i-1]+outB[j*sx+i+1]-outG[j*sx+i+1])/2;
      for (j=1;j<sy-2;j+=2)
	outB[j*sx+i]=(outB[(j-1)*sx+i-1]-outG[(j-1)*sx+i-1]+outB[(j+1)*sx+i-1]-outG[(j+1)*sx+i-1]+
		      outB[(j-1)*sx+i+1]-outG[(j-1)*sx+i+1]+outB[(j+1)*sx+i+1]-outG[(j+1)*sx+i+1])/4;
    }
}
