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

// this should disappear...
extern void swab();

#define CLIP(in, out)\
   in = in < 0 ? 0 : in;\
   in = in > 255 ? 255 : in;\
   out=in;
  
/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/

void
yuyv2uyvy(unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    swab(src, dest, NumPixels << 1);
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    memcpy(dest,src, NumPixels<<1);
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
uyvy2yuyv(unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    swab(src, dest, NumPixels << 1);
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    memcpy(dest,src, NumPixels<<1);
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}
void
uyyvyy2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  register int i=NumPixels + (NumPixels >> 1)-1;
  register int j=(NumPixels << 1)-1;
  register int y0, y1, y2, y3, u, v;

  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    while (i >= 0) {
      y3 = src[i--];
      y2 = src[i--];
      v  = src[i--];
      y1 = src[i--];
      y0 = src[i--];
      u  = src[i--];

      dest[j--] = v;
      dest[j--] = y3;
      dest[j--] = u;
      dest[j--] = y2;
      
      dest[j--] = v;
      dest[j--] = y1;
      dest[j--] = u;
      dest[j--] = y0;
    }
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    while (i >= 0) {
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
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }

}

void
uyv2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  register int i = NumPixels + (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1;

  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    while (i >= 0) {
      v1 = src[i--];
      y1 = src[i--];
      u1 = src[i--];
      v0 = src[i--];
      y0 = src[i--];
      u0 = src[i--];

      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y0;
    }
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    while (i >= 0) {
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
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
y2uyvy (unsigned char *src, unsigned char *dest, 
	unsigned long src_width, unsigned long src_height,
	unsigned long dest_pitch, int byte_order)
{
  if ((src_width*2) == dest_pitch) {
    // do it the quick way
    register int i = src_width*src_height - 1;
    register int j = (src_width*src_height << 1) - 1;
    register int y0, y1;
    
    switch (byte_order) {
    case OVERLAY_BYTE_ORDER_YUYV:
      while (i >= 0) {
	y1 = src[i--];
	y0 = src[i--];
	dest[j--] = 128;
	dest[j--] = y1;
	dest[j--] = 128;
	dest[j--] = y0;
      }
      break;
    case OVERLAY_BYTE_ORDER_UYVY:
      while (i >= 0) {
	y1 = src[i--];
	y0 = src[i--];
	dest[j--] = y1;
	dest[j--] = 128;
	dest[j--] = y0;
	dest[j--] = 128;
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
  } else { // src_width*2 != dest_pitch
    register int x, y;

    //assert ((dest_pitch - 2*src_width)==1);

    switch (byte_order) {
    case OVERLAY_BYTE_ORDER_YUYV:
      y=src_height;
      while (y--) {
	x=src_width;
	while (x--) {
	  *dest++ = *src++;
	  *dest++ = 128;
	}
	// padding required, duplicate last column
	*dest++ = *(src-1);
	*dest++ = 128;
      }
      break;
    case OVERLAY_BYTE_ORDER_UYVY:
      y=src_height;
      while (y--) {
	x=src_width;
	while (x--) {
	  *dest++ = 128;
	  *dest++ = *src++;
	}
	// padding required, duplicate last column
	*dest++ = 128;
	*dest++ = *(src-1);
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
  }
}

void
y162uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits, int byte_order)
{
  register int i = (NumPixels << 1)-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1;

  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    while (i >= 0) {
      y1 = src[i--];
      y1 = (y1 + (((int)src[i--])<<8))>>(bits-8);
      y0 = src[i--];
      y0 = (y0 + (((int)src[i--])<<8))>>(bits-8);
      dest[j--] = 128;
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
    }
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    while (i >= 0) {
      y1 = src[i--];
      y1 = (y1 + (((int)src[i--])<<8))>>(bits-8);
      y0 = src[i--];
      y0 = (y0 + (((int)src[i--])<<8))>>(bits-8);
      dest[j--] = y1;
      dest[j--] = 128;
      dest[j--] = y0;
      dest[j--] = 128;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }

}

void
y162y (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits)
{
  register int i = (NumPixels<<1)-1;
  register int j = NumPixels-1;
  register int y;

  while (i >= 0) {
    y = src[i--];
    dest[j--] = (y + (src[i--]<<8))>>(bits-8);
  }
}

void
rgb2uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  register int i = NumPixels + ( NumPixels << 1 )-1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    while (i >= 0) {
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y1;
    }
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    while (i >= 0) {
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y0, u0 , v0);
      b = (unsigned char) src[i--];
      g = (unsigned char) src[i--];
      r = (unsigned char) src[i--];
      RGB2YUV (r, g, b, y1, u1 , v1);
      dest[j--] = y0;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

void
rgb482uyvy (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int byte_order)
{
  register int i = ( (NumPixels + ( NumPixels << 1 )) << 1 ) -1;
  register int j = (NumPixels << 1)-1;
  register int y0, y1, u0, u1, v0, v1 ;
  register int r, g, b;

  switch (byte_order) {
  case OVERLAY_BYTE_ORDER_YUYV:
    while (i >= 0) {
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
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y0;
      dest[j--] = (u0+u1) >> 1;
      dest[j--] = y1;
    } 
    break;
  case OVERLAY_BYTE_ORDER_UYVY:
    while (i >= 0) {
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
      dest[j--] = y0;
      dest[j--] = (v0+v1) >> 1;
      dest[j--] = y1;
      dest[j--] = (u0+u1) >> 1;
    }
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
}

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO RGB 24bpp 
 *
 **********************************************************************/

void
rgb482rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = ((NumPixels + ( NumPixels << 1 )) << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;

  while (i >= 0) {
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
    i--;
    dest[j--]=src[i--];
  }
}


void
uyv2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = NumPixels + ( NumPixels << 1 ) -1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y, u, v;
  register int r, g, b;

  while (i >= 0) {
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
uyvy2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 ) -1;
  register int y0, y1, u, v;
  register int r, g, b;

  while (i >= 0) {
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
uyyvyy2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = NumPixels + ( NumPixels >> 1 )-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y0, y1, y2, y3, u, v;
  register int r, g, b;
  
  while (i >= 0) {
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
y2rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = NumPixels-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i >= 0) {
    y = (unsigned char) src[i--];
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

void
y162rgb (unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits)
{
  register int i = (NumPixels << 1)-1;
  register int j = NumPixels + ( NumPixels << 1 )-1;
  register int y;

  while (i > 0) {
    y = src[i--];
    y = (y + (src[i--]<<8))>>(bits-8);
    dest[j--] = y;
    dest[j--] = y;
    dest[j--] = y;
  }
}

/*****************************************************************
 *     Color conversion functions for cameras that can           * 
 * output raw-Bayer pattern images, such as some Basler and      *
 * Point Grey camera. Most of the algos presented here come from *
 * http://ise0.Stanford.EDU/~tingchen/main.htm and have been     *
 * converted from Matlab to C and extended to all elementary     *
 * patterns.                                                     *
 *****************************************************************/

void
BayerNearestNeighbor(unsigned char *src, unsigned char *dest, int sx, int sy, int type)
{
  unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
  register int i,j;

  // sx and sy should be even
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG:
  case COLOR_FILTER_FORMAT7_BGGR:
    outR=&dest[0];
    outG=&dest[1];
    outB=&dest[2];
    break;
  case COLOR_FILTER_FORMAT7_GBRG:
  case COLOR_FILTER_FORMAT7_RGGB:
    outR=&dest[2];
    outG=&dest[1];
    outB=&dest[0];
    break;
  default:
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
  
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG: //-------------------------------------------
  case COLOR_FILTER_FORMAT7_GBRG:
    // copy original RGB data to output images
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	outG[(i*sx+j)*3]=src[i*sx+j];
	outG[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
	outR[(i*sx+j+1)*3]=src[i*sx+j+1];
	outB[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
      }
    }
    // R channel
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	outR[(i*sx+j)*3]=outR[(i*sx+j+1)*3];
	outR[((i+1)*sx+j+1)*3]=outR[(i*sx+j+1)*3];
	outR[((i+1)*sx+j)*3]=outR[(i*sx+j+1)*3];
      }
    }
      // B channel
    for (i=0;i<sy-1;i+=2)  { //every two lines
      for (j=0;j<sx-1;j+=2) {
	outB[(i*sx+j)*3]=outB[((i+1)*sx+j)*3];
	outB[(i*sx+j+1)*3]=outB[((i+1)*sx+j)*3];
	outB[((i+1)*sx+j+1)*3]=outB[((i+1)*sx+j)*3];
      }
    }
    // using lower direction for G channel
      
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=1;j<sx;j+=2)
	outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];
      
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      outG[((sy-1)*sx+j)*3]=outG[((sy-2)*sx+j)*3];
    
    break;
  case COLOR_FILTER_FORMAT7_BGGR: //-------------------------------------------
  case COLOR_FILTER_FORMAT7_RGGB:
    // copy original data
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	outB[(i*sx+j)*3]=src[i*sx+j];
	outR[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
	outG[(i*sx+j+1)*3]=src[i*sx+j+1];
	outG[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
      }
    }
    // R channel
    for (i=0;i<sy;i+=2){
      for (j=0;j<sx-1;j+=2) {
	outR[(i*sx+j)*3]=outR[((i+1)*sx+j+1)*3];
	outR[(i*sx+j+1)*3]=outR[((i+1)*sx+j+1)*3];
	outR[((i+1)*sx+j)*3]=outR[((i+1)*sx+j+1)*3];
      }
    }
    // B channel
    for (i=0;i<sy-1;i+=2) { //every two lines
      for (j=0;j<sx-1;j+=2) {
	outB[((i+1)*sx+j)*3]=outB[(i*sx+j)*3];
	outB[(i*sx+j+1)*3]=outB[(i*sx+j)*3];
	outB[((i+1)*sx+j+1)*3]=outB[(i*sx+j)*3];
      }
    }
    // using lower direction for G channel
    
    // G channel
    for (i=0;i<sy-1;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	outG[(i*sx+j)*3]=outG[((i+1)*sx+j)*3];
    
    for (i=1;i<sy-2;i+=2)//every two lines
      for (j=0;j<sx-1;j+=2)
	outG[(i*sx+j+1)*3]=outG[((i+1)*sx+j+1)*3];
    
    // copy it for the next line
    for (j=0;j<sx-1;j+=2)
      outG[((sy-1)*sx+j+1)*3]=outG[((sy-2)*sx+j+1)*3];
    
    break;
    
  default:  //-------------------------------------------
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
}


void
BayerEdgeSense(unsigned char *src, unsigned char *dest, int sx, int sy, int type)
{
  unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
  register int i,j;
  int dh, dv;
  int tmp;

  // sx and sy should be even
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG:
  case COLOR_FILTER_FORMAT7_BGGR:
    outR=&dest[0];
    outG=&dest[1];
    outB=&dest[2];
    break;
  case COLOR_FILTER_FORMAT7_GBRG:
  case COLOR_FILTER_FORMAT7_RGGB:
    outR=&dest[2];
    outG=&dest[1];
    outB=&dest[0];
    break;
  default:
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }

  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG://---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_GBRG:
    // copy original RGB data to output images
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	outG[(i*sx+j)*3]=src[i*sx+j];
	outG[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
	outR[(i*sx+j+1)*3]=src[i*sx+j+1];
	outB[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
      }
    }
    // process GREEN channel
    for (i=3;i<sy-2;i+=2) {
      for (j=2;j<sx-3;j+=2) {
	dh=abs((outB[(i*sx+j-2)*3]+outB[(i*sx+j+2)*3])/2-outB[(i*sx+j)*3]);
	dv=abs((outB[((i-2)*sx+j)*3]+outB[((i+2)*sx+j)*3])/2-outB[(i*sx+j)*3]);
	if (dh<dv)
	  tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3])/2;
	else {
	  if (dh>dv)
	    tmp=(outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/2;
	  else
	    tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3]+outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/4;
	}
	CLIP(tmp,outG[(i*sx+j)*3]);
      }
    }

    for (i=2;i<sy-3;i+=2) {
      for (j=3;j<sx-2;j+=2) {
	dh=abs((outR[(i*sx+j-2)*3]+outR[(i*sx+j+2)*3])/2-outR[(i*sx+j)*3]);
	dv=abs((outR[((i-2)*sx+j)*3]+outR[((i+2)*sx+j)*3])/2-outR[(i*sx+j)*3]);
	if (dh<dv)
	  tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3])/2;
	else {
	  if (dh>dv)
	    tmp=(outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/2;
	  else
	    tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3]+outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/4;
	} 
	CLIP(tmp,outG[(i*sx+j)*3]);
      }
    }
    // process RED channel
    for (i=0;i<sy-1;i+=2) {
      for (j=2;j<sx-1;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outR[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
			      outR[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
    }
    for (i=1;i<sy-2;i+=2) {
      for (j=1;j<sx;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
			      outR[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
      for (j=2;j<sx-1;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
			      outR[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
			      outR[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
			      outR[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
    }
      
    // process BLUE channel
    for (i=1;i<sy;i+=2) {
      for (j=1;j<sx-2;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
			      outB[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
    }
    for (i=2;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
			      outB[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
      for (j=1;j<sx-2;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
			      outB[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
			      outB[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
			      outB[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
    }
      break;

  case COLOR_FILTER_FORMAT7_BGGR: //---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_RGGB:
    // copy original RGB data to output images
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	outB[(i*sx+j)*3]=src[i*sx+j];
	outR[((i+1)*sx+(j+1))*3]=src[(i+1)*sx+(j+1)];
	outG[(i*sx+j+1)*3]=src[i*sx+j+1];
	outG[((i+1)*sx+j)*3]=src[(i+1)*sx+j];
      }
    }
    // process GREEN channel
    for (i=2;i<sy-2;i+=2) {
      for (j=2;j<sx-3;j+=2) {
	dh=abs((outB[(i*sx+j-2)*3]+outB[(i*sx+j+2)*3])/2-outB[(i*sx+j)*3]);
	dv=abs((outB[((i-2)*sx+j)*3]+outB[((i+2)*sx+j)*3])/2-outB[(i*sx+j)*3]);
	if (dh<dv)
	  tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3])/2;
	else {
	  if (dh>dv)
	    tmp=(outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/2;
	  else
	    tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3]+outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/4;
	}
	CLIP(tmp,outG[(i*sx+j)*3]);
      }
    }
    for (i=3;i<sy-3;i+=2) {
      for (j=3;j<sx-2;j+=2) {
	dh=abs((outR[(i*sx+j-2)*3]+outR[(i*sx+j+2)*3])/2-outR[(i*sx+j)*3]);
	dv=abs((outR[((i-2)*sx+j)*3]+outR[((i+2)*sx+j)*3])/2-outR[(i*sx+j)*3]);
	if (dh<dv)
	  tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3])/2;
	else {
	  if (dh>dv)
	    tmp=(outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/2;
	  else
	    tmp=(outG[(i*sx+j-1)*3]+outG[(i*sx+j+1)*3]+outG[((i-1)*sx+j)*3]+outG[((i+1)*sx+j)*3])/4;
	}
	CLIP(tmp,outG[(i*sx+j)*3]);
      }
    }
    // process RED channel
    for (i=1;i<sy-1;i+=2) { // G-points (1/2)
      for (j=2;j<sx-1;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outR[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
			      outR[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
    }
    for (i=2;i<sy-2;i+=2)  {
      for (j=1;j<sx;j+=2) { // G-points (2/2)
	tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
			      outR[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
      for (j=2;j<sx-1;j+=2) { // B-points
	tmp=outG[(i*sx+j)*3]+(outR[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
			      outR[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
			      outR[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
			      outR[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
	CLIP(tmp,outR[(i*sx+j)*3]);
      }
    }
    
      // process BLUE channel
    for (i=0;i<sy;i+=2) {
      for (j=1;j<sx-2;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[(i*sx+j-1)*3]-outG[(i*sx+j-1)*3]+
			      outB[(i*sx+j+1)*3]-outG[(i*sx+j+1)*3])/2;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
    }
    for (i=1;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j)*3]-outG[((i-1)*sx+j)*3]+
			      outB[((i+1)*sx+j)*3]-outG[((i+1)*sx+j)*3])/2;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
      for (j=1;j<sx-2;j+=2) {
	tmp=outG[(i*sx+j)*3]+(outB[((i-1)*sx+j-1)*3]-outG[((i-1)*sx+j-1)*3]+
			      outB[((i-1)*sx+j+1)*3]-outG[((i-1)*sx+j+1)*3]+
			      outB[((i+1)*sx+j-1)*3]-outG[((i+1)*sx+j-1)*3]+
			      outB[((i+1)*sx+j+1)*3]-outG[((i+1)*sx+j+1)*3])/4;
	CLIP(tmp,outB[(i*sx+j)*3]);
      }
    }
    break;
  default: //---------------------------------------------------------
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
  
  ClearBorders(dest, sx, sy, 3);

}

void
ClearBorders(unsigned char* dest, int sx, int sy, int w) 
{
  int i,j;

  // black edges:
  i=3*sx*w-1;
  j=3*sx*(sy-1)-1;
  while (i>=0) {
    dest[i--]=0;
    dest[j--]=0;
  }
  
  i=sx*(sy-1)*3-1+w*3;
  while (i>sx) {
    j=6*w;
    while (j>0) {
      dest[i--]=0;
      j--;
    }
    i-=(sx-2*w)*3;
  }
  
}


void
BayerDownsample(unsigned char *src, unsigned char *dest, int sx, int sy, int type)
{
  unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
  register int i,j;
  int tmp;

  sx*=2;
  sy*=2;

  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG:
  case COLOR_FILTER_FORMAT7_BGGR:
    outR=&dest[0];
    outG=&dest[1];
    outB=&dest[2];
    break;
  case COLOR_FILTER_FORMAT7_GBRG:
  case COLOR_FILTER_FORMAT7_RGGB:
    outR=&dest[2];
    outG=&dest[1];
    outB=&dest[0];
    break;
  default:
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
  
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG://---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_GBRG:
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	tmp=((src[i*sx+j]+src[(i+1)*sx+(j+1)])>>1);
	CLIP(tmp,outG[(((i*sx)>>2)+(j>>1))*3]);
	tmp=src[i*sx+j+1];
	CLIP(tmp,outR[(((i*sx)>>2)+(j>>1))*3]);
	tmp=src[(i+1)*sx+j];
	CLIP(tmp,outB[(((i*sx)>>2)+(j>>1))*3]);
      }
    }
    break;
  case COLOR_FILTER_FORMAT7_BGGR://---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_RGGB:
    for (i=0;i<sy;i+=2) {
      for (j=0;j<sx;j+=2) {
	tmp=((src[(i+1)*sx+j]+src[i*sx+(j+1)])>>1);
	CLIP(tmp,outG[(((i*sx)>>2)+(j>>1))*3]);
	tmp=src[(i+1)*sx+j+1];
	CLIP(tmp,outR[(((i*sx)>>2)+(j>>1))*3]);
	tmp=src[i*sx+j];
	CLIP(tmp,outB[(((i*sx)>>2)+(j>>1))*3]);
      }
    }
    break;
  default: //---------------------------------------------------------
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
  
}

void
BayerSimple(unsigned char *src, unsigned char *dest, int sx, int sy, int type)
{
  unsigned char *outR=NULL, *outG=NULL, *outB=NULL;
  register int i,j;
  int tmp, base;
  
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG:
  case COLOR_FILTER_FORMAT7_BGGR:
    outR=&dest[0];
    outG=&dest[1];
    outB=&dest[2];
    break;
  case COLOR_FILTER_FORMAT7_GBRG:
  case COLOR_FILTER_FORMAT7_RGGB:
    outR=&dest[2];
    outG=&dest[1];
    outB=&dest[0];
    break;
  default:
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
  
  switch (type) {
  case COLOR_FILTER_FORMAT7_GRBG://---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_GBRG:
    for (i=0;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base]+src[base+sx+1])>>1);
	CLIP(tmp,outG[base*3]);
	tmp=src[base+1];
	CLIP(tmp,outR[base*3]);
	tmp=src[base+sx];
	CLIP(tmp,outB[base*3]);
      }
    }
    for (i=0;i<sy-1;i+=2) {
      for (j=1;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base+1]+src[base+sx])>>1);
	CLIP(tmp,outG[(base)*3]);
	tmp=src[base];
	CLIP(tmp,outR[(base)*3]);
	tmp=src[base+1+sx];
	CLIP(tmp,outB[(base)*3]);
      }
    }
    for (i=1;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base+sx]+src[base+1])>>1);
	CLIP(tmp,outG[base*3]);
	tmp=src[base+sx+1];
	CLIP(tmp,outR[base*3]);
	tmp=src[base];
	CLIP(tmp,outB[base*3]);
      }
    }
    for (i=1;i<sy-1;i+=2) {
      for (j=1;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base]+src[base+1+sx])>>1);
	CLIP(tmp,outG[(base)*3]);
	tmp=src[base+sx];
	CLIP(tmp,outR[(base)*3]);
	tmp=src[base+1];
	CLIP(tmp,outB[(base)*3]);
      }
    }
    break;
  case COLOR_FILTER_FORMAT7_BGGR://---------------------------------------------------------
  case COLOR_FILTER_FORMAT7_RGGB:
    for (i=0;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base+sx]+src[base+1])>>1);
	CLIP(tmp,outG[base*3]);
	tmp=src[base+sx+1];
	CLIP(tmp,outR[base*3]);
	tmp=src[base];
	CLIP(tmp,outB[base*3]);
      }
    }
    for (i=1;i<sy-1;i+=2) {
      for (j=0;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base]+src[base+1+sx])>>1);
	CLIP(tmp,outG[(base)*3]);
	tmp=src[base+1];
	CLIP(tmp,outR[(base)*3]);
	tmp=src[base+sx];
	CLIP(tmp,outB[(base)*3]);
      }
    }
    for (i=0;i<sy-1;i+=2) {
      for (j=1;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base]+src[base+sx+1])>>1);
	CLIP(tmp,outG[base*3]);
	tmp=src[base+sx];
	CLIP(tmp,outR[base*3]);
	tmp=src[base+1];
	CLIP(tmp,outB[base*3]);
      }
    }
    for (i=1;i<sy-1;i+=2) {
      for (j=1;j<sx-1;j+=2) {
	base=i*sx+j;
	tmp=((src[base+1]+src[base+sx])>>1);
	CLIP(tmp,outG[(base)*3]);
	tmp=src[base];
	CLIP(tmp,outR[(base)*3]);
	tmp=src[base+1+sx];
	CLIP(tmp,outB[(base)*3]);
      }
    }
    break;
  default: //---------------------------------------------------------
    fprintf(stderr,"Bad Bayer pattern ID: %d\n",type);
    return;
    break;
  }
 
  // add black border
  for (i=sx*(sy-1)*3;i<sx*sy*3;i++) {
    dest[i]=0;
  }
  for (i=(sx-1)*3;i<sx*sy*3;i+=(sx-1)*3) {
    dest[i++]=0;
    dest[i++]=0;
    dest[i++]=0;
  }


}

// change a 16bit stereo image (8bit/channel) into two 8bit images on top
// of each other
void
StereoDecode(unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
  register int i = NumPixels-1;
  register int j = (NumPixels>>1)-1;
  register int k = NumPixels-1;

  while (i >= 0) {
    dest[k--] = src[i--];
    dest[j--] = src[i--];
  }
}
