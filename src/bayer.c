/*
 * Copyright (C) 2000-2005 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Frederic Devernay
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

#define uint16_t unsigned short int

//#define LEGACY_CORIANDER

#define CLIP(in, out)\
   in = in < 0 ? 0 : in;\
   in = in > 255 ? 255 : in;\
   out=in;
  
#define CLIP16(in, out, bits)\
   in = in < 0 ? 0 : in;\
   in = in > ((1<<bits)-1) ? ((1<<bits)-1) : in;\
   out=in;
  
/**************************************************************
 *     Color conversion functions for cameras that can        * 
 * output raw-Bayer pattern images, such as some Basler and   *
 * Point Grey camera. Most of the algos presented here come   *
 * from http://www-ise.stanford.edu/~tingchen/ and have been  *
 * converted from Matlab to C and extended to all elementary  *
 * patterns.                                                  *
 **************************************************************/

/**************************************************************
 *  Benchmarks of Bayer pattern decoding                      *
 *                                                            *
 * (in % of processor use)                                    *
 * Test conditions: - Point Grey Dragonfly, 640x480, 30fps    *
 *                  - only service running is RECEIVE         *
 * 		 - Dell Precision 370, P4, 2.8GHz, 1GB RAM    *
 *                                                            *
 * type		  std	  +march=i686/mcpu=i686               *
 * ---------------------------------------------              *
 * none		   0.6	   0.8                                *
 * nearest	   2.4	   2.1                                *
 * simple	   2.4	   2.4                                *
 * edge sense 2	  30.2	  27.0                                *
 * downsample	   2.5	   2.7                                *
 * bilinear	   3.2	   3.2                                *
 * HQ Linear	  16.0	  16.0                                *
 **************************************************************/

/* 8-bits versions */
#ifndef LEGACY_CORIANDER
/* insprired by OpenCV's Bayer decoding */
void
BayerNearestNeighbor(const unsigned char *bayer, unsigned char *dst,
		     int sx, int sy, int code)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;
    int i, imax, iinc;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
	dst[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
	dst[i++] = 0;
	dst[i++] = 0;
	dst[i++] = 0;
    }

    dst += 1;
    width -= 1;
    height -= 1;

    for (; height--; bayer += bayerStep, dst += dstStep) {
      //int t0, t1;
	const unsigned char *bayerEnd = bayer + width;

        if (start_with_green) {
            dst[-blue] = bayer[1];
            dst[0] = bayer[bayerStep + 1];
            dst[blue] = bayer[bayerStep];
            bayer++;
            dst += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[-1] = bayer[0];
                dst[0] = bayer[1];
                dst[1] = bayer[bayerStep + 1];

                dst[2] = bayer[2];
                dst[3] = bayer[bayerStep + 2];
                dst[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[1] = bayer[0];
                dst[0] = bayer[1];
                dst[-1] = bayer[bayerStep + 1];

                dst[4] = bayer[2];
                dst[3] = bayer[bayerStep + 2];
                dst[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            dst[-blue] = bayer[0];
            dst[0] = bayer[1];
            dst[blue] = bayer[bayerStep + 1];
            bayer++;
            dst += 3;
        }

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}
#else
/* coriander's Bayer decoding (GPL) */
void
BayerNearestNeighbor(const unsigned char *src, unsigned char *dest, int sx,
		     int sy, int type)
{
    unsigned char *outR, *outG, *outB;
    register int i, j;

    /* sx and sy should be even */
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	outR = NULL;
	outG = NULL;
	outB = NULL;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_GBRG:
	/* copy original RGB data to output images */
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outG[(i * sx + j) * 3] = src[i * sx + j];
		outG[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outR[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outB[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	/* R channel */
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		outR[(i * sx + j) * 3] = outR[(i * sx + j + 1) * 3];
		outR[((i + 1) * sx + j + 1) * 3] =
		    outR[(i * sx + j + 1) * 3];
		outR[((i + 1) * sx + j) * 3] = outR[(i * sx + j + 1) * 3];
	    }
	}
	/* B channel */
	for (i = 0; i < sy - 1; i += 2) {	/* every two lines */
	    for (j = 0; j < sx - 1; j += 2) {
		outB[(i * sx + j) * 3] = outB[((i + 1) * sx + j) * 3];
		outB[(i * sx + j + 1) * 3] = outB[((i + 1) * sx + j) * 3];
		outB[((i + 1) * sx + j + 1) * 3] =
		    outB[((i + 1) * sx + j) * 3];
	    }
	}
	/* using lower direction for G channel */

	/* G channel */
	for (i = 0; i < sy - 1; i += 2)	/* every two lines */
	    for (j = 1; j < sx; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	for (i = 1; i < sy - 2; i += 2)	/* every two lines */
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	/* copy it for the next line */
	for (j = 0; j < sx - 1; j += 2)
	    outG[((sy - 1) * sx + j) * 3] = outG[((sy - 2) * sx + j) * 3];

	break;
    case COLOR_FILTER_FORMAT7_BGGR:
    case COLOR_FILTER_FORMAT7_RGGB:
	/* copy original data */
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outB[(i * sx + j) * 3] = src[i * sx + j];
		outR[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outG[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outG[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	// R channel
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		outR[(i * sx + j) * 3] = outR[((i + 1) * sx + j + 1) * 3];
		outR[(i * sx + j + 1) * 3] =
		    outR[((i + 1) * sx + j + 1) * 3];
		outR[((i + 1) * sx + j) * 3] =
		    outR[((i + 1) * sx + j + 1) * 3];
	    }
	}
	// B channel
	for (i = 0; i < sy - 1; i += 2) {	//every two lines
	    for (j = 0; j < sx - 1; j += 2) {
		outB[((i + 1) * sx + j) * 3] = outB[(i * sx + j) * 3];
		outB[(i * sx + j + 1) * 3] = outB[(i * sx + j) * 3];
		outB[((i + 1) * sx + j + 1) * 3] = outB[(i * sx + j) * 3];
	    }
	}
	// using lower direction for G channel

	// G channel
	for (i = 0; i < sy - 1; i += 2)	//every two lines
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	for (i = 1; i < sy - 2; i += 2)	//every two lines
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j + 1) * 3] =
		    outG[((i + 1) * sx + j + 1) * 3];

	// copy it for the next line
	for (j = 0; j < sx - 1; j += 2)
	    outG[((sy - 1) * sx + j + 1) * 3] =
		outG[((sy - 2) * sx + j + 1) * 3];

	break;

    default:			//-------------------------------------------
	break;
    }
}
#endif				/* OpenCV vs. Coriander */

void
ClearBorders(unsigned char *dest, int sx, int sy, int w)
{
    int i, j;

    // black edges:
    i = 3 * sx * w - 1;
    j = 3 * sx * sy - 1;
    while (i >= 0) {
	dest[i--] = 0;
	dest[j--] = 0;
    }

    i = sx * (sy - 1) * 3 - 1 + w * 3;
    while (i > sx) {
	j = 6 * w;
	while (j > 0) {
	    dest[i--] = 0;
	    j--;
	}
	i -= (sx - 2 * w) * 3;
    }

}

/* OpenCV's Bayer decoding */
void
BayerBilinear(const unsigned char *bayer, unsigned char *dst, int sx,
	      int sy, int code)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    /*
       the two letters  of the OpenCV name are respectively
       the 4th and 3rd letters from the blinky name,
       and we also have to switch R and B (OpenCV is BGR)

       CV_BayerBG2BGR <-> COLOR_FILTER_FORMAT7_BGGR
       CV_BayerGB2BGR <-> COLOR_FILTER_FORMAT7_GBRG
       CV_BayerGR2BGR <-> COLOR_FILTER_FORMAT7_GRBG

       int blue = code == CV_BayerBG2BGR || code == CV_BayerGB2BGR ? -1 : 1;
       int start_with_green = code == CV_BayerGB2BGR || code == CV_BayerGR2BGR;
     */
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;

    ClearBorders(dst, sx, sy, 1);
    dst += dstStep + 3 + 1;
    height -= 2;
    width -= 2;

    for (; height--; bayer += bayerStep, dst += dstStep) {
	int t0, t1;
	const unsigned char *bayerEnd = bayer + width;

	if (start_with_green) {
	    /* OpenCV has a bug in the next line, which was
	       t0 = (bayer[0] + bayer[bayerStep * 2] + 1) >> 1; */
	    t0 = (bayer[1] + bayer[bayerStep * 2 + 1] + 1) >> 1;
	    t1 = (bayer[bayerStep] + bayer[bayerStep + 2] + 1) >> 1;
	    dst[-blue] = (unsigned char) t0;
	    dst[0] = bayer[bayerStep + 1];
	    dst[blue] = (unsigned char) t1;
	    bayer++;
	    dst += 3;
	}

	if (blue > 0) {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		      bayer[bayerStep * 2 + 2] + 2) >> 2;
		t1 = (bayer[1] + bayer[bayerStep] +
		      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		      2) >> 2;
		dst[-1] = (unsigned char) t0;
		dst[0] = (unsigned char) t1;
		dst[1] = bayer[bayerStep + 1];

		t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
		t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		      1) >> 1;
		dst[2] = (unsigned char) t0;
		dst[3] = bayer[bayerStep + 2];
		dst[4] = (unsigned char) t1;
	    }
	} else {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		      bayer[bayerStep * 2 + 2] + 2) >> 2;
		t1 = (bayer[1] + bayer[bayerStep] +
		      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		      2) >> 2;
		dst[1] = (unsigned char) t0;
		dst[0] = (unsigned char) t1;
		dst[-1] = bayer[bayerStep + 1];

		t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
		t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		      1) >> 1;
		dst[4] = (unsigned char) t0;
		dst[3] = bayer[bayerStep + 2];
		dst[2] = (unsigned char) t1;
	    }
	}

	if (bayer < bayerEnd) {
	    t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		  bayer[bayerStep * 2 + 2] + 2) >> 2;
	    t1 = (bayer[1] + bayer[bayerStep] +
		  bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		  2) >> 2;
	    dst[-blue] = (unsigned char) t0;
	    dst[0] = (unsigned char) t1;
	    dst[blue] = bayer[bayerStep + 1];
	    bayer++;
	    dst += 3;
	}

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}

/* High-Quality Linear Interpolation For Demosaicing Of
   Bayer-Patterned Color Images, by Henrique S. Malvar, Li-wei He, and
   Ross Cutler, in ICASSP'04 */
void
BayerHQLinear(const unsigned char *bayer, unsigned char *dst, int sx,
	      int sy, int code)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;

    ClearBorders(dst, sx, sy, 2);
    dst += 2 * dstStep + 6 + 1;
    height -= 4;
    width -= 4;

    /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
    blue = -blue;

    for (; height--; bayer += bayerStep, dst += dstStep) {
	int t0, t1;
	const unsigned char *bayerEnd = bayer + width;
	const int bayerStep2 = bayerStep * 2;
	const int bayerStep3 = bayerStep * 3;
	const int bayerStep4 = bayerStep * 4;

	if (start_with_green) {
	    /* at green pixel */
	    dst[0] = bayer[bayerStep2 + 2];
	    t0 = dst[0] * 5
		+ ((bayer[bayerStep + 2] + bayer[bayerStep3 + 2]) << 2)
		- bayer[2]
		- bayer[bayerStep + 1]
		- bayer[bayerStep + 3]
		- bayer[bayerStep3 + 1]
		- bayer[bayerStep3 + 3]
		- bayer[bayerStep4 + 2]
		+ ((bayer[bayerStep2] + bayer[bayerStep2 + 4] + 1) >> 1);
	    t1 = dst[0] * 5 +
		((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 3]) << 2)
		- bayer[bayerStep2]
		- bayer[bayerStep + 1]
		- bayer[bayerStep + 3]
		- bayer[bayerStep3 + 1]
		- bayer[bayerStep3 + 3]
		- bayer[bayerStep2 + 4]
		+ ((bayer[2] + bayer[bayerStep4 + 2] + 1) >> 1);
	    t0 = (t0 + 4) >> 3;
	    CLIP(t0, dst[-blue]);
	    t1 = (t1 + 4) >> 3;
	    CLIP(t1, dst[blue]);
	    bayer++;
	    dst += 3;
	}

	if (blue > 0) {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		/* B at B */
		dst[1] = bayer[bayerStep2 + 2];
		/* R at B */
		t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
		    -
		    (((bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						     2]) * 3 + 1) >> 1)
		    + dst[1] * 6;
		/* G at B */
		t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		       bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2]) << 1)
		    - (bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		    + (dst[1] << 2);
		t0 = (t0 + 4) >> 3;
		CLIP(t0, dst[-1]);
		t1 = (t1 + 4) >> 3;
		CLIP(t1, dst[0]);
		/* at green pixel */
		dst[3] = bayer[bayerStep2 + 3];
		t0 = dst[3] * 5
		    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
		    - bayer[3]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep4 + 3]
		    +
		    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
		      1) >> 1);
		t1 = dst[3] * 5 +
		    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
		    - bayer[bayerStep2 + 1]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep2 + 5]
		    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
		t0 = (t0 + 4) >> 3;
		CLIP(t0, dst[2]);
		t1 = (t1 + 4) >> 3;
		CLIP(t1, dst[4]);
	    }
	} else {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		/* R at R */
		dst[-1] = bayer[bayerStep2 + 2];
		/* B at R */
		t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
		    -
		    (((bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						     2]) * 3 + 1) >> 1)
		    + dst[-1] * 6;
		/* G at R */
		t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		       bayer[bayerStep2 + 3] + bayer[bayerStep * 3 +
						     2]) << 1)
		    - (bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		    + (dst[-1] << 2);
		t0 = (t0 + 4) >> 3;
		CLIP(t0, dst[1]);
		t1 = (t1 + 4) >> 3;
		CLIP(t1, dst[0]);

		/* at green pixel */
		dst[3] = bayer[bayerStep2 + 3];
		t0 = dst[3] * 5
		    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
		    - bayer[3]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep4 + 3]
		    +
		    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
		      1) >> 1);
		t1 = dst[3] * 5 +
		    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
		    - bayer[bayerStep2 + 1]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep2 + 5]
		    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
		t0 = (t0 + 4) >> 3;
		CLIP(t0, dst[4]);
		t1 = (t1 + 4) >> 3;
		CLIP(t1, dst[2]);
	    }
	}

	if (bayer < bayerEnd) {
	    /* B at B */
	    dst[blue] = bayer[bayerStep2 + 2];
	    /* R at B */
	    t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
		-
		(((bayer[2] + bayer[bayerStep2] +
		   bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						 2]) * 3 + 1) >> 1)
		+ dst[blue] * 6;
	    /* G at B */
	    t1 = (((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		    bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2])) << 1)
		- (bayer[2] + bayer[bayerStep2] +
		   bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		+ (dst[blue] << 2);
	    t0 = (t0 + 4) >> 3;
	    CLIP(t0, dst[-blue]);
	    t1 = (t1 + 4) >> 3;
	    CLIP(t1, dst[0]);
	    bayer++;
	    dst += 3;
	}

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}

/* coriander's Bayer decoding (GPL) */
/* Edge Sensing Interpolation II from http://www-ise.stanford.edu/~tingchen/ */
/*   (Laroche,Claude A.  "Apparatus and method for adaptively
     interpolating a full color image utilizing chrominance gradients"
     U.S. Patent 5,373,322) */
void
BayerEdgeSense(const unsigned char *src, unsigned char *dest, int sx,
	       int sy, int type)
{
    unsigned char *outR, *outG, *outB;
    register int i, j;
    int dh, dv;
    int tmp;

    // sx and sy should be even
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	// copy original RGB data to output images
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 0; j < sx; j += 2) {
	  outG[(i + j) * 3] = src[i + j];
	  outG[(i + sx + (j + 1)) * 3] = src[i + sx + (j + 1)];
	  outR[(i + j + 1) * 3] = src[i + j + 1];
	  outB[(i + sx + j) * 3] = src[i + sx + j];
	}
      }
      // process GREEN channel
      for (i = 3*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 3; j += 2) {
	  dh = abs(((outB[(i + j - 2) * 3] +
		     outB[(i + j + 2) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  dv = abs(((outB[(i - (sx<<1) + j) * 3] +
		     outB[(i + (sx<<1) + j) * 3]) >> 1)  -
		   outB[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP(tmp, outG[(i + j) * 3]);
	}
      }
	
      for (i = 2*sx; i < (sy - 3)*sx; i += (sx<<1)) {
	for (j = 3; j < sx - 2; j += 2) {
	  dh = abs(((outR[(i + j - 2) * 3] +
		     outR[(i + j + 2) * 3]) >>1 ) -
		   outR[(i + j) * 3]);
	  dv = abs(((outR[(i - (sx<<1) + j) * 3] +
		     outR[(i + (sx<<1) + j) * 3]) >>1 ) -
		   outR[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP(tmp, outG[(i + j) * 3]);
	}
      }
      // process RED channel
      for (i = 0; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outR[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
      }
      for (i = sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 1; j < sx; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outR[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outR[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outR[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outR[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
      }

      // process BLUE channel
      for (i = sx; i < sy*sx; i += (sx<<1)) {
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outB[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
      }
      for (i = 2*sx; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 0; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outB[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outB[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outB[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outB[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
      }
      break;

    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	// copy original RGB data to output images
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 0; j < sx; j += 2) {
	  outB[(i + j) * 3] = src[i + j];
	  outR[(i + sx + (j + 1)) * 3] = src[i + sx + (j + 1)];
	  outG[(i + j + 1) * 3] = src[i + j + 1];
	  outG[(i + sx + j) * 3] = src[i + sx + j];
	}
      }
      // process GREEN channel
      for (i = 2*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 3; j += 2) {
	  dh = abs(((outB[(i + j - 2) * 3] +
		    outB[(i + j + 2) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  dv = abs(((outB[(i - (sx<<1) + j) * 3] +
		    outB[(i + (sx<<1) + j) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP(tmp, outG[(i + j) * 3]);
	}
      }
      for (i = 3*sx; i < (sy - 3)*sx; i += (sx<<1)) {
	for (j = 3; j < sx - 2; j += 2) {
	  dh = abs(((outR[(i + j - 2) * 3] +
		    outR[(i + j + 2) * 3]) >> 1) -
		   outR[(i + j) * 3]);
	  dv = abs(((outR[(i - (sx<<1) + j) * 3] +
		    outR[(i + (sx<<1) + j) * 3]) >> 1) -
		   outR[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >>1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >>1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >>2;
	  }
	  CLIP(tmp, outG[(i + j) * 3]);
	}
      }
      // process RED channel
      for (i = sx; i < (sy - 1)*sx; i += (sx<<1)) {	// G-points (1/2)
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outR[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >>1);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
      }
      for (i = 2*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 1; j < sx; j += 2) {	// G-points (2/2)
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outR[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
	for (j = 2; j < sx - 1; j += 2) {	// B-points
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outR[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outR[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outR[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP(tmp, outR[(i + j) * 3]);
	}
      }
      
      // process BLUE channel
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outB[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
      }
      for (i = sx; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 0; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outB[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outB[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outB[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outB[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP(tmp, outB[(i + j) * 3]);
	}
      }
      break;
    default:			//---------------------------------------------------------
      fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
      return;
      break;
    }
    
    ClearBorders(dest, sx, sy, 3);
}

/* coriander's Bayer decoding (GPL) */
void
BayerDownsample(const unsigned char *src, unsigned char *dest, int sx,
		int sy, int type)
{
    unsigned char *outR, *outG, *outB;
    register int i, j;
    int tmp;

    sx *= 2;
    sy *= 2;

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad Bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	for (i = 0; i < sy*sx; i += (sx<<1)) {
	    for (j = 0; j < sx; j += 2) {
		tmp =
		    ((src[i + j] + src[i + sx + j + 1]) >> 1);
		CLIP(tmp, outG[((i >> 2) + (j >> 1)) * 3]);
		tmp = src[i + sx + j + 1];
		CLIP(tmp, outR[((i >> 2) + (j >> 1)) * 3]);
		tmp = src[i + sx + j];
		CLIP(tmp, outB[((i >> 2) + (j >> 1)) * 3]);
	    }
	}
	break;
    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	for (i = 0; i < sy*sx; i += (sx<<1)) {
	    for (j = 0; j < sx; j += 2) {
		tmp =
		    ((src[i + sx + j] + src[i + j + 1]) >> 1);
		CLIP(tmp, outG[((i >> 2) + (j >> 1)) * 3]);
		tmp = src[i + sx + j + 1];
		CLIP(tmp, outR[((i >> 2) + (j >> 1)) * 3]);
		tmp = src[i + j];
		CLIP(tmp, outB[((i >> 2) + (j >> 1)) * 3]);
	    }
	}
	break;
    default:			//---------------------------------------------------------
	fprintf(stderr, "Bad Bayer pattern ID: %d\n", type);
	return;
	break;
    }

}

/* coriander's Bayer decoding (GPL) */
#ifndef LEGACY_CORIANDER
void
BayerSimple(const unsigned char *bayer, unsigned char *dst,
            int sx, int sy, int code)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
        || code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
        || code == COLOR_FILTER_FORMAT7_GRBG;
    int i, imax, iinc;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
        dst[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
        dst[i++] = 0;
        dst[i++] = 0;
        dst[i++] = 0;
    }

    dst += 1;
    width -= 1;
    height -= 1;

    for (; height--; bayer += bayerStep, dst += dstStep) {
        const unsigned char *bayerEnd = bayer + width;

        if (start_with_green) {
            dst[-blue] = bayer[1];
            dst[0] = (bayer[0] + bayer[bayerStep + 1] + 1) >> 1;
            dst[blue] = bayer[bayerStep];
            bayer++;
            dst += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[-1] = bayer[0];
                dst[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                dst[1] = bayer[bayerStep + 1];

                dst[2] = bayer[2];
                dst[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                dst[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[1] = bayer[0];
                dst[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
                dst[-1] = bayer[bayerStep + 1];

                dst[4] = bayer[2];
                dst[3] = (bayer[1] + bayer[bayerStep + 2] + 1) >> 1;
                dst[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            dst[-blue] = bayer[0];
            dst[0] = (bayer[1] + bayer[bayerStep] + 1) >> 1;
            dst[blue] = bayer[bayerStep + 1];
            bayer++;
            dst += 3;
        }

        bayer -= width;
        dst -= width * 3;

        blue = -blue;
        start_with_green = !start_with_green;
    }
}

#else
void
BayerSimple(const unsigned char *src, unsigned char *dest, int sx, int sy,
	    int type)
{
    unsigned char *outR, *outG, *outB;
    register int i, j;
    int tmp, base;

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	outR = NULL;
	outG = NULL;
	outB = NULL;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + sx + 1]) >> 1);
		CLIP(tmp, outG[base * 3]);
		tmp = src[base + 1];
		CLIP(tmp, outR[base * 3]);
		tmp = src[base + sx];
		CLIP(tmp, outB[base * 3]);
	    }
	}
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + 1] + src[base + sx]) >> 1);
		CLIP(tmp, outG[(base) * 3]);
		tmp = src[base];
		CLIP(tmp, outR[(base) * 3]);
		tmp = src[base + 1 + sx];
		CLIP(tmp, outB[(base) * 3]);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + sx] + src[base + 1]) >> 1);
		CLIP(tmp, outG[base * 3]);
		tmp = src[base + sx + 1];
		CLIP(tmp, outR[base * 3]);
		tmp = src[base];
		CLIP(tmp, outB[base * 3]);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + 1 + sx]) >> 1);
		CLIP(tmp, outG[(base) * 3]);
		tmp = src[base + sx];
		CLIP(tmp, outR[(base) * 3]);
		tmp = src[base + 1];
		CLIP(tmp, outB[(base) * 3]);
	    }
	}
	break;
    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + sx] + src[base + 1]) >> 1);
		CLIP(tmp, outG[base * 3]);
		tmp = src[base + sx + 1];
		CLIP(tmp, outR[base * 3]);
		tmp = src[base];
		CLIP(tmp, outB[base * 3]);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + 1 + sx]) >> 1);
		CLIP(tmp, outG[(base) * 3]);
		tmp = src[base + 1];
		CLIP(tmp, outR[(base) * 3]);
		tmp = src[base + sx];
		CLIP(tmp, outB[(base) * 3]);
	    }
	}
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + sx + 1]) >> 1);
		CLIP(tmp, outG[base * 3]);
		tmp = src[base + sx];
		CLIP(tmp, outR[base * 3]);
		tmp = src[base + 1];
		CLIP(tmp, outB[base * 3]);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + 1] + src[base + sx]) >> 1);
		CLIP(tmp, outG[(base) * 3]);
		tmp = src[base];
		CLIP(tmp, outR[(base) * 3]);
		tmp = src[base + 1 + sx];
		CLIP(tmp, outB[(base) * 3]);
	    }
	}
	break;
    default:			//---------------------------------------------------------
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    /* add black border */
    for (i = sx * (sy - 1) * 3; i < sx * sy * 3; i++) {
	dest[i] = 0;
    }
    for (i = (sx - 1) * 3; i < sx * sy * 3; i += (sx - 1) * 3) {
	dest[i++] = 0;
	dest[i++] = 0;
	dest[i++] = 0;
    }
}

#endif

/* 16-bits versions */

#ifndef LEGACY_CORIANDER
/* insprired by OpenCV's Bayer decoding */
void
BayerNearestNeighbor_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
			    int sy, int code, int bits)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;
    int i, iinc, imax;

    /* add black border */
    imax = sx * sy * 3;
    for (i = sx * (sy - 1) * 3; i < imax; i++) {
	dst[i] = 0;
    }
    iinc = (sx - 1) * 3;
    for (i = (sx - 1) * 3; i < imax; i += iinc) {
	dst[i++] = 0;
	dst[i++] = 0;
	dst[i++] = 0;
    }

    dst += 1;
    height -= 1;
    width -= 1;

    for (; height--; bayer += bayerStep, dst += dstStep) {
      //int t0, t1;
	const uint16_t *bayerEnd = bayer + width;

        if (start_with_green) {
            dst[-blue] = bayer[1];
            dst[0] = bayer[bayerStep + 1];
            dst[blue] = bayer[bayerStep];
            bayer++;
            dst += 3;
        }

        if (blue > 0) {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[-1] = bayer[0];
                dst[0] = bayer[1];
                dst[1] = bayer[bayerStep + 1];

                dst[2] = bayer[2];
                dst[3] = bayer[bayerStep + 2];
                dst[4] = bayer[bayerStep + 1];
            }
        } else {
            for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
                dst[1] = bayer[0];
                dst[0] = bayer[1];
                dst[-1] = bayer[bayerStep + 1];

                dst[4] = bayer[2];
                dst[3] = bayer[bayerStep + 2];
                dst[2] = bayer[bayerStep + 1];
            }
        }

        if (bayer < bayerEnd) {
            dst[-blue] = bayer[0];
            dst[0] = bayer[1];
            dst[blue] = bayer[bayerStep + 1];
            bayer++;
            dst += 3;
        }

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}
#else
/* coriander's Bayer decoding (GPL) */
void
BayerNearestNeighbor_uint16(const uint16_t * src, uint16_t * dest, int sx,
			    int sy, int type, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;

    // sx and sy should be even
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	outR = NULL;
	outG = NULL;
	outB = NULL;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//-------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	// copy original RGB data to output images
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outG[(i * sx + j) * 3] = src[i * sx + j];
		outG[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outR[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outB[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	// R channel
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		outR[(i * sx + j) * 3] = outR[(i * sx + j + 1) * 3];
		outR[((i + 1) * sx + j + 1) * 3] =
		    outR[(i * sx + j + 1) * 3];
		outR[((i + 1) * sx + j) * 3] = outR[(i * sx + j + 1) * 3];
	    }
	}
	// B channel
	for (i = 0; i < sy - 1; i += 2) {	//every two lines
	    for (j = 0; j < sx - 1; j += 2) {
		outB[(i * sx + j) * 3] = outB[((i + 1) * sx + j) * 3];
		outB[(i * sx + j + 1) * 3] = outB[((i + 1) * sx + j) * 3];
		outB[((i + 1) * sx + j + 1) * 3] =
		    outB[((i + 1) * sx + j) * 3];
	    }
	}
	// using lower direction for G channel

	// G channel
	for (i = 0; i < sy - 1; i += 2)	//every two lines
	    for (j = 1; j < sx; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	for (i = 1; i < sy - 2; i += 2)	//every two lines
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	// copy it for the next line
	for (j = 0; j < sx - 1; j += 2)
	    outG[((sy - 1) * sx + j) * 3] = outG[((sy - 2) * sx + j) * 3];

	break;
    case COLOR_FILTER_FORMAT7_BGGR:	//-------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	// copy original data
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outB[(i * sx + j) * 3] = src[i * sx + j];
		outR[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outG[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outG[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	// R channel
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		outR[(i * sx + j) * 3] = outR[((i + 1) * sx + j + 1) * 3];
		outR[(i * sx + j + 1) * 3] =
		    outR[((i + 1) * sx + j + 1) * 3];
		outR[((i + 1) * sx + j) * 3] =
		    outR[((i + 1) * sx + j + 1) * 3];
	    }
	}
	// B channel
	for (i = 0; i < sy - 1; i += 2) {	//every two lines
	    for (j = 0; j < sx - 1; j += 2) {
		outB[((i + 1) * sx + j) * 3] = outB[(i * sx + j) * 3];
		outB[(i * sx + j + 1) * 3] = outB[(i * sx + j) * 3];
		outB[((i + 1) * sx + j + 1) * 3] = outB[(i * sx + j) * 3];
	    }
	}
	// using lower direction for G channel

	// G channel
	for (i = 0; i < sy - 1; i += 2)	//every two lines
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j) * 3] = outG[((i + 1) * sx + j) * 3];

	for (i = 1; i < sy - 2; i += 2)	//every two lines
	    for (j = 0; j < sx - 1; j += 2)
		outG[(i * sx + j + 1) * 3] =
		    outG[((i + 1) * sx + j + 1) * 3];

	// copy it for the next line
	for (j = 0; j < sx - 1; j += 2)
	    outG[((sy - 1) * sx + j + 1) * 3] =
		outG[((sy - 2) * sx + j + 1) * 3];

	break;

    default:			//-------------------------------------------
	break;
    }
}
#endif				/* OpenCV vs. Coriander */

void
ClearBorders_uint16(uint16_t * dest, int sx, int sy, int w)
{
    int i, j;

    // black edges:
    i = 3 * sx * w - 1;
    j = 3 * sx * sy - 1;
    while (i >= 0) {
	dest[i--] = 0;
	dest[j--] = 0;
    }

    i = sx * (sy - 1) * 3 - 1 + w * 3;
    while (i > sx) {
	j = 6 * w;
	while (j > 0) {
	    dest[i--] = 0;
	    j--;
	}
	i -= (sx - 2 * w) * 3;
    }

}

/* OpenCV's Bayer decoding */
void
BayerBilinear_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
		     int sy, int code, int bits)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;

    dst += dstStep + 3 + 1;
    height -= 2;
    width -= 2;

    for (; height--; bayer += bayerStep, dst += dstStep) {
	int t0, t1;
	const uint16_t *bayerEnd = bayer + width;

	if (start_with_green) {
	    /* OpenCV has a bug in the next line, which was
	       t0 = (bayer[0] + bayer[bayerStep * 2] + 1) >> 1; */
	    t0 = (bayer[1] + bayer[bayerStep * 2 + 1] + 1) >> 1;
	    t1 = (bayer[bayerStep] + bayer[bayerStep + 2] + 1) >> 1;
	    dst[-blue] = (uint16_t) t0;
	    dst[0] = bayer[bayerStep + 1];
	    dst[blue] = (uint16_t) t1;
	    bayer++;
	    dst += 3;
	}

	if (blue > 0) {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		      bayer[bayerStep * 2 + 2] + 2) >> 2;
		t1 = (bayer[1] + bayer[bayerStep] +
		      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		      2) >> 2;
		dst[-1] = (uint16_t) t0;
		dst[0] = (uint16_t) t1;
		dst[1] = bayer[bayerStep + 1];

		t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
		t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		      1) >> 1;
		dst[2] = (uint16_t) t0;
		dst[3] = bayer[bayerStep + 2];
		dst[4] = (uint16_t) t1;
	    }
	} else {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		      bayer[bayerStep * 2 + 2] + 2) >> 2;
		t1 = (bayer[1] + bayer[bayerStep] +
		      bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		      2) >> 2;
		dst[1] = (uint16_t) t0;
		dst[0] = (uint16_t) t1;
		dst[-1] = bayer[bayerStep + 1];

		t0 = (bayer[2] + bayer[bayerStep * 2 + 2] + 1) >> 1;
		t1 = (bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		      1) >> 1;
		dst[4] = (uint16_t) t0;
		dst[3] = bayer[bayerStep + 2];
		dst[2] = (uint16_t) t1;
	    }
	}

	if (bayer < bayerEnd) {
	    t0 = (bayer[0] + bayer[2] + bayer[bayerStep * 2] +
		  bayer[bayerStep * 2 + 2] + 2) >> 2;
	    t1 = (bayer[1] + bayer[bayerStep] +
		  bayer[bayerStep + 2] + bayer[bayerStep * 2 + 1] +
		  2) >> 2;
	    dst[-blue] = (uint16_t) t0;
	    dst[0] = (uint16_t) t1;
	    dst[blue] = bayer[bayerStep + 1];
	    bayer++;
	    dst += 3;
	}

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}

/* High-Quality Linear Interpolation For Demosaicing Of
   Bayer-Patterned Color Images, by Henrique S. Malvar, Li-wei He, and
   Ross Cutler, in ICASSP'04 */
void
BayerHQLinear_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
		     int sy, int code, int bits)
{
    const int bayerStep = sx;
    const int dstStep = 3 * sx;
    int width = sx;
    int height = sy;
    /*
       the two letters  of the OpenCV name are respectively
       the 4th and 3rd letters from the blinky name,
       and we also have to switch R and B (OpenCV is BGR)

       CV_BayerBG2BGR <-> COLOR_FILTER_FORMAT7_BGGR
       CV_BayerGB2BGR <-> COLOR_FILTER_FORMAT7_GBRG
       CV_BayerGR2BGR <-> COLOR_FILTER_FORMAT7_GRBG

       int blue = code == CV_BayerBG2BGR || code == CV_BayerGB2BGR ? -1 : 1;
       int start_with_green = code == CV_BayerGB2BGR || code == CV_BayerGR2BGR;
     */
    int blue = code == COLOR_FILTER_FORMAT7_BGGR
	|| code == COLOR_FILTER_FORMAT7_GBRG ? -1 : 1;
    int start_with_green = code == COLOR_FILTER_FORMAT7_GBRG
	|| code == COLOR_FILTER_FORMAT7_GRBG;

    ClearBorders_uint16(dst, sx, sy, 2);
    dst += 2 * dstStep + 6 + 1;
    height -= 4;
    width -= 4;

    /* We begin with a (+1 line,+1 column) offset with respect to bilinear decoding, so start_with_green is the same, but blue is opposite */
    blue = -blue;

    for (; height--; bayer += bayerStep, dst += dstStep) {
	int t0, t1;
	const uint16_t *bayerEnd = bayer + width;
	const int bayerStep2 = bayerStep * 2;
	const int bayerStep3 = bayerStep * 3;
	const int bayerStep4 = bayerStep * 4;

	if (start_with_green) {
	    /* at green pixel */
	    dst[0] = bayer[bayerStep2 + 2];
	    t0 = dst[0] * 5
		+ ((bayer[bayerStep + 2] + bayer[bayerStep3 + 2]) << 2)
		- bayer[2]
		- bayer[bayerStep + 1]
		- bayer[bayerStep + 3]
		- bayer[bayerStep3 + 1]
		- bayer[bayerStep3 + 3]
		- bayer[bayerStep4 + 2]
		+ ((bayer[bayerStep2] + bayer[bayerStep2 + 4] + 1) >> 1);
	    t1 = dst[0] * 5 +
		((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 3]) << 2)
		- bayer[bayerStep2]
		- bayer[bayerStep + 1]
		- bayer[bayerStep + 3]
		- bayer[bayerStep3 + 1]
		- bayer[bayerStep3 + 3]
		- bayer[bayerStep2 + 4]
		+ ((bayer[2] + bayer[bayerStep4 + 2] + 1) >> 1);
	    t0 = (t0 + 4) >> 3;
	    CLIP16(t0, dst[-blue], bits);
	    t1 = (t1 + 4) >> 3;
	    CLIP16(t1, dst[blue], bits);
	    bayer++;
	    dst += 3;
	}

	if (blue > 0) {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		/* B at B */
		dst[1] = bayer[bayerStep2 + 2];
		/* R at B */
		t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		       bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
		    -
		    (((bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						     2]) * 3 + 1) >> 1)
		    + dst[1] * 6;
		/* G at B */
		t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		       bayer[bayerStep2 + 3] + bayer[bayerStep * 3 +
						     2]) << 1)
		    - (bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		    + (dst[1] << 2);
		t0 = (t0 + 4) >> 3;
		CLIP16(t0, dst[-1], bits);
		t1 = (t1 + 4) >> 3;
		CLIP16(t1, dst[0], bits);
		/* at green pixel */
		dst[3] = bayer[bayerStep2 + 3];
		t0 = dst[3] * 5
		    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
		    - bayer[3]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep4 + 3]
		    +
		    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
		      1) >> 1);
		t1 = dst[3] * 5 +
		    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
		    - bayer[bayerStep2 + 1]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep2 + 5]
		    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
		t0 = (t0 + 4) >> 3;
		CLIP16(t0, dst[2], bits);
		t1 = (t1 + 4) >> 3;
		CLIP16(t1, dst[4], bits);
	    }
	} else {
	    for (; bayer <= bayerEnd - 2; bayer += 2, dst += 6) {
		/* R at R */
		dst[-1] = bayer[bayerStep2 + 2];
		/* B at R */
		t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		       bayer[bayerStep * 3 + 1] + bayer[bayerStep3 +
							3]) << 1)
		    -
		    (((bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						     2]) * 3 + 1) >> 1)
		    + dst[-1] * 6;
		/* G at R */
		t1 = ((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		       bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2]) << 1)
		    - (bayer[2] + bayer[bayerStep2] +
		       bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		    + (dst[-1] << 2);
		t0 = (t0 + 4) >> 3;
		CLIP16(t0, dst[1], bits);
		t1 = (t1 + 4) >> 3;
		CLIP16(t1, dst[0], bits);

		/* at green pixel */
		dst[3] = bayer[bayerStep2 + 3];
		t0 = dst[3] * 5
		    + ((bayer[bayerStep + 3] + bayer[bayerStep3 + 3]) << 2)
		    - bayer[3]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep4 + 3]
		    +
		    ((bayer[bayerStep2 + 1] + bayer[bayerStep2 + 5] +
		      1) >> 1);
		t1 = dst[3] * 5 +
		    ((bayer[bayerStep2 + 2] + bayer[bayerStep2 + 4]) << 2)
		    - bayer[bayerStep2 + 1]
		    - bayer[bayerStep + 2]
		    - bayer[bayerStep + 4]
		    - bayer[bayerStep3 + 2]
		    - bayer[bayerStep3 + 4]
		    - bayer[bayerStep2 + 5]
		    + ((bayer[3] + bayer[bayerStep4 + 3] + 1) >> 1);
		t0 = (t0 + 4) >> 3;
		CLIP16(t0, dst[4], bits);
		t1 = (t1 + 4) >> 3;
		CLIP16(t1, dst[2], bits);
	    }
	}

	if (bayer < bayerEnd) {
	    /* B at B */
	    dst[blue] = bayer[bayerStep2 + 2];
	    /* R at B */
	    t0 = ((bayer[bayerStep + 1] + bayer[bayerStep + 3] +
		   bayer[bayerStep3 + 1] + bayer[bayerStep3 + 3]) << 1)
		-
		(((bayer[2] + bayer[bayerStep2] +
		   bayer[bayerStep2 + 4] + bayer[bayerStep4 +
						 2]) * 3 + 1) >> 1)
		+ dst[blue] * 6;
	    /* G at B */
	    t1 = (((bayer[bayerStep + 2] + bayer[bayerStep2 + 1] +
		    bayer[bayerStep2 + 3] + bayer[bayerStep3 + 2])) << 1)
		- (bayer[2] + bayer[bayerStep2] +
		   bayer[bayerStep2 + 4] + bayer[bayerStep4 + 2])
		+ (dst[blue] << 2);
	    t0 = (t0 + 4) >> 3;
	    CLIP16(t0, dst[-blue], bits);
	    t1 = (t1 + 4) >> 3;
	    CLIP16(t1, dst[0], bits);
	    bayer++;
	    dst += 3;
	}

	bayer -= width;
	dst -= width * 3;

	blue = -blue;
	start_with_green = !start_with_green;
    }
}

/* coriander's Bayer decoding (GPL) */
void
BayerEdgeSense_uint16(const uint16_t * src, uint16_t * dest, int sx,
		      int sy, int type, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;
    int dh, dv;
    int tmp;

    // sx and sy should be even
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	// copy original RGB data to output images
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 0; j < sx; j += 2) {
	  outG[(i + j) * 3] = src[i + j];
	  outG[(i + sx + (j + 1)) * 3] = src[i + sx + (j + 1)];
	  outR[(i + j + 1) * 3] = src[i + j + 1];
	  outB[(i + sx + j) * 3] = src[i + sx + j];
	}
      }
      // process GREEN channel
      for (i = 3*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 3; j += 2) {
	  dh = abs(((outB[(i + j - 2) * 3] +
		     outB[(i + j + 2) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  dv = abs(((outB[(i - (sx<<1) + j) * 3] +
		     outB[(i + (sx<<1) + j) * 3]) >> 1)  -
		   outB[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
	
      for (i = 2*sx; i < (sy - 3)*sx; i += (sx<<1)) {
	for (j = 3; j < sx - 2; j += 2) {
	  dh = abs(((outR[(i + j - 2) * 3] +
		     outR[(i + j + 2) * 3]) >>1 ) -
		   outR[(i + j) * 3]);
	  dv = abs(((outR[(i - (sx<<1) + j) * 3] +
		     outR[(i + (sx<<1) + j) * 3]) >>1 ) -
		   outR[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      // process RED channel
      for (i = 0; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outR[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      for (i = sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 1; j < sx; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outR[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outR[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outR[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outR[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }

      // process BLUE channel
      for (i = sx; i < sy*sx; i += (sx<<1)) {
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outB[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      for (i = 2*sx; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 0; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outB[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outB[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outB[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outB[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      break;

    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	// copy original RGB data to output images
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 0; j < sx; j += 2) {
	  outB[(i + j) * 3] = src[i + j];
	  outR[(i + sx + (j + 1)) * 3] = src[i + sx + (j + 1)];
	  outG[(i + j + 1) * 3] = src[i + j + 1];
	  outG[(i + sx + j) * 3] = src[i + sx + j];
	}
      }
      // process GREEN channel
      for (i = 2*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 2; j < sx - 3; j += 2) {
	  dh = abs(((outB[(i + j - 2) * 3] +
		    outB[(i + j + 2) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  dv = abs(((outB[(i - (sx<<1) + j) * 3] +
		    outB[(i + (sx<<1) + j) * 3]) >> 1) -
		   outB[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >> 1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >> 2;
	  }
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      for (i = 3*sx; i < (sy - 3)*sx; i += (sx<<1)) {
	for (j = 3; j < sx - 2; j += 2) {
	  dh = abs(((outR[(i + j - 2) * 3] +
		    outR[(i + j + 2) * 3]) >> 1) -
		   outR[(i + j) * 3]);
	  dv = abs(((outR[(i - (sx<<1) + j) * 3] +
		    outR[(i + (sx<<1) + j) * 3]) >> 1) -
		   outR[(i + j) * 3]);
	  if (dh < dv)
	    tmp = (outG[(i + j - 1) * 3] +
		   outG[(i + j + 1) * 3]) >>1;
	  else {
	    if (dh > dv)
	      tmp = (outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >>1;
	    else
	      tmp = (outG[(i + j - 1) * 3] +
		     outG[(i + j + 1) * 3] +
		     outG[(i - sx + j) * 3] +
		     outG[(i + sx + j) * 3]) >>2;
	  }
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      // process RED channel
      for (i = sx; i < (sy - 1)*sx; i += (sx<<1)) {	// G-points (1/2)
	for (j = 2; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outR[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >>1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      for (i = 2*sx; i < (sy - 2)*sx; i += (sx<<1)) {
	for (j = 1; j < sx; j += 2) {	// G-points (2/2)
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outR[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
	for (j = 2; j < sx - 1; j += 2) {	// B-points
	  tmp = outG[(i + j) * 3] +
	      ((outR[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outR[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outR[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outR[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      
      // process BLUE channel
      for (i = 0; i < sy*sx; i += (sx<<1)) {
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i + j - 1) * 3] -
		outG[(i + j - 1) * 3] +
		outB[(i + j + 1) * 3] -
		outG[(i + j + 1) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      for (i = sx; i < (sy - 1)*sx; i += (sx<<1)) {
	for (j = 0; j < sx - 1; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j) * 3] -
		outG[(i - sx + j) * 3] +
		outB[(i + sx + j) * 3] -
		outG[(i + sx + j) * 3]) >> 1);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
	for (j = 1; j < sx - 2; j += 2) {
	  tmp = outG[(i + j) * 3] +
	      ((outB[(i - sx + j - 1) * 3] -
		outG[(i - sx + j - 1) * 3] +
		outB[(i - sx + j + 1) * 3] -
		outG[(i - sx + j + 1) * 3] +
		outB[(i + sx + j - 1) * 3] -
		outG[(i + sx + j - 1) * 3] +
		outB[(i + sx + j + 1) * 3] -
		outG[(i + sx + j + 1) * 3]) >> 2);
	  CLIP16(tmp, outR[(i + j) * 3], bits);
	}
      }
      break;
    default:			//---------------------------------------------------------
      fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
      return;
      break;
    }
   
    /*
    // sx and sy should be even
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	// copy original RGB data to output images
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outG[(i * sx + j) * 3] = src[i * sx + j];
		outG[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outR[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outB[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	// process GREEN channel
	for (i = 3; i < sy - 2; i += 2) {
	    for (j = 2; j < sx - 3; j += 2) {
		dh = abs((outB[(i * sx + j - 2) * 3] +
			  outB[(i * sx + j + 2) * 3]) / 2 - outB[(i * sx +
								  j) * 3]);
		dv = abs((outB[((i - 2) * sx + j) * 3] +
			  outB[((i + 2) * sx + j) * 3]) / 2 -
			 outB[(i * sx + j) * 3]);
		if (dh < dv)
		    tmp =
			(outG[(i * sx + j - 1) * 3] +
			 outG[(i * sx + j + 1) * 3]) / 2;
		else {
		    if (dh > dv)
			tmp =
			    (outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 2;
		    else
			tmp =
			    (outG[(i * sx + j - 1) * 3] +
			     outG[(i * sx + j + 1) * 3] +
			     outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 4;
		}
		CLIP16(tmp, outG[(i * sx + j) * 3], bits);
	    }
	}

	for (i = 2; i < sy - 3; i += 2) {
	    for (j = 3; j < sx - 2; j += 2) {
		dh = abs((outR[(i * sx + j - 2) * 3] +
			  outR[(i * sx + j + 2) * 3]) / 2 - outR[(i * sx +
								  j) * 3]);
		dv = abs((outR[((i - 2) * sx + j) * 3] +
			  outR[((i + 2) * sx + j) * 3]) / 2 -
			 outR[(i * sx + j) * 3]);
		if (dh < dv)
		    tmp =
			(outG[(i * sx + j - 1) * 3] +
			 outG[(i * sx + j + 1) * 3]) / 2;
		else {
		    if (dh > dv)
			tmp =
			    (outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 2;
		    else
			tmp =
			    (outG[(i * sx + j - 1) * 3] +
			     outG[(i * sx + j + 1) * 3] +
			     outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 4;
		}
		CLIP16(tmp, outG[(i * sx + j) * 3], bits);
	    }
	}
	// process RED channel
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 2; j < sx - 1; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] + (outR[(i * sx + j - 1) * 3] -
					      outG[(i * sx + j - 1) * 3] +
					      outR[(i * sx + j + 1) * 3] -
					      outG[(i * sx + j +
						    1) * 3]) / 2;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	}
	for (i = 1; i < sy - 2; i += 2) {
	    for (j = 1; j < sx; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outR[((i - 1) * sx + j) * 3] -
		     outG[((i - 1) * sx + j) * 3] +
		     outR[((i + 1) * sx + j) * 3] -
		     outG[((i + 1) * sx + j) * 3]) / 2;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	    for (j = 2; j < sx - 1; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outR[((i - 1) * sx + j - 1) * 3] -
		     outG[((i - 1) * sx + j - 1) * 3] +
		     outR[((i - 1) * sx + j + 1) * 3] -
		     outG[((i - 1) * sx + j + 1) * 3] +
		     outR[((i + 1) * sx + j - 1) * 3] -
		     outG[((i + 1) * sx + j - 1) * 3] +
		     outR[((i + 1) * sx + j + 1) * 3] -
		     outG[((i + 1) * sx + j + 1) * 3]) / 4;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	}

	// process BLUE channel
	for (i = 1; i < sy; i += 2) {
	    for (j = 1; j < sx - 2; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] + (outB[(i * sx + j - 1) * 3] -
					      outG[(i * sx + j - 1) * 3] +
					      outB[(i * sx + j + 1) * 3] -
					      outG[(i * sx + j +
						    1) * 3]) / 2;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	}
	for (i = 2; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outB[((i - 1) * sx + j) * 3] -
		     outG[((i - 1) * sx + j) * 3] +
		     outB[((i + 1) * sx + j) * 3] -
		     outG[((i + 1) * sx + j) * 3]) / 2;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	    for (j = 1; j < sx - 2; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outB[((i - 1) * sx + j - 1) * 3] -
		     outG[((i - 1) * sx + j - 1) * 3] +
		     outB[((i - 1) * sx + j + 1) * 3] -
		     outG[((i - 1) * sx + j + 1) * 3] +
		     outB[((i + 1) * sx + j - 1) * 3] -
		     outG[((i + 1) * sx + j - 1) * 3] +
		     outB[((i + 1) * sx + j + 1) * 3] -
		     outG[((i + 1) * sx + j + 1) * 3]) / 4;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	}
	break;

    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	// copy original RGB data to output images
	for (i = 0; i < sy; i += 2) {
	    for (j = 0; j < sx; j += 2) {
		outB[(i * sx + j) * 3] = src[i * sx + j];
		outR[((i + 1) * sx + (j + 1)) * 3] =
		    src[(i + 1) * sx + (j + 1)];
		outG[(i * sx + j + 1) * 3] = src[i * sx + j + 1];
		outG[((i + 1) * sx + j) * 3] = src[(i + 1) * sx + j];
	    }
	}
	// process GREEN channel
	for (i = 2; i < sy - 2; i += 2) {
	    for (j = 2; j < sx - 3; j += 2) {
		dh = abs((outB[(i * sx + j - 2) * 3] +
			  outB[(i * sx + j + 2) * 3]) / 2 - outB[(i * sx +
								  j) * 3]);
		dv = abs((outB[((i - 2) * sx + j) * 3] +
			  outB[((i + 2) * sx + j) * 3]) / 2 -
			 outB[(i * sx + j) * 3]);
		if (dh < dv)
		    tmp =
			(outG[(i * sx + j - 1) * 3] +
			 outG[(i * sx + j + 1) * 3]) / 2;
		else {
		    if (dh > dv)
			tmp =
			    (outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 2;
		    else
			tmp =
			    (outG[(i * sx + j - 1) * 3] +
			     outG[(i * sx + j + 1) * 3] +
			     outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 4;
		}
		CLIP16(tmp, outG[(i * sx + j) * 3], bits);
	    }
	}
	for (i = 3; i < sy - 3; i += 2) {
	    for (j = 3; j < sx - 2; j += 2) {
		dh = abs((outR[(i * sx + j - 2) * 3] +
			  outR[(i * sx + j + 2) * 3]) / 2 - outR[(i * sx +
								  j) * 3]);
		dv = abs((outR[((i - 2) * sx + j) * 3] +
			  outR[((i + 2) * sx + j) * 3]) / 2 -
			 outR[(i * sx + j) * 3]);
		if (dh < dv)
		    tmp =
			(outG[(i * sx + j - 1) * 3] +
			 outG[(i * sx + j + 1) * 3]) / 2;
		else {
		    if (dh > dv)
			tmp =
			    (outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 2;
		    else
			tmp =
			    (outG[(i * sx + j - 1) * 3] +
			     outG[(i * sx + j + 1) * 3] +
			     outG[((i - 1) * sx + j) * 3] +
			     outG[((i + 1) * sx + j) * 3]) / 4;
		}
		CLIP16(tmp, outG[(i * sx + j) * 3], bits);
	    }
	}
	// process RED channel
	for (i = 1; i < sy - 1; i += 2) {	// G-points (1/2)
	    for (j = 2; j < sx - 1; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] + (outR[(i * sx + j - 1) * 3] -
					      outG[(i * sx + j - 1) * 3] +
					      outR[(i * sx + j + 1) * 3] -
					      outG[(i * sx + j +
						    1) * 3]) / 2;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	}
	for (i = 2; i < sy - 2; i += 2) {
	    for (j = 1; j < sx; j += 2) {	// G-points (2/2)
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outR[((i - 1) * sx + j) * 3] -
		     outG[((i - 1) * sx + j) * 3] +
		     outR[((i + 1) * sx + j) * 3] -
		     outG[((i + 1) * sx + j) * 3]) / 2;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	    for (j = 2; j < sx - 1; j += 2) {	// B-points
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outR[((i - 1) * sx + j - 1) * 3] -
		     outG[((i - 1) * sx + j - 1) * 3] +
		     outR[((i - 1) * sx + j + 1) * 3] -
		     outG[((i - 1) * sx + j + 1) * 3] +
		     outR[((i + 1) * sx + j - 1) * 3] -
		     outG[((i + 1) * sx + j - 1) * 3] +
		     outR[((i + 1) * sx + j + 1) * 3] -
		     outG[((i + 1) * sx + j + 1) * 3]) / 4;
		CLIP16(tmp, outR[(i * sx + j) * 3], bits);
	    }
	}

	// process BLUE channel
	for (i = 0; i < sy; i += 2) {
	    for (j = 1; j < sx - 2; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] + (outB[(i * sx + j - 1) * 3] -
					      outG[(i * sx + j - 1) * 3] +
					      outB[(i * sx + j + 1) * 3] -
					      outG[(i * sx + j +
						    1) * 3]) / 2;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outB[((i - 1) * sx + j) * 3] -
		     outG[((i - 1) * sx + j) * 3] +
		     outB[((i + 1) * sx + j) * 3] -
		     outG[((i + 1) * sx + j) * 3]) / 2;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	    for (j = 1; j < sx - 2; j += 2) {
		tmp =
		    outG[(i * sx + j) * 3] +
		    (outB[((i - 1) * sx + j - 1) * 3] -
		     outG[((i - 1) * sx + j - 1) * 3] +
		     outB[((i - 1) * sx + j + 1) * 3] -
		     outG[((i - 1) * sx + j + 1) * 3] +
		     outB[((i + 1) * sx + j - 1) * 3] -
		     outG[((i + 1) * sx + j - 1) * 3] +
		     outB[((i + 1) * sx + j + 1) * 3] -
		     outG[((i + 1) * sx + j + 1) * 3]) / 4;
		CLIP16(tmp, outB[(i * sx + j) * 3], bits);
	    }
	}
	break;
    default:			//---------------------------------------------------------
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }
    */
    ClearBorders_uint16(dest, sx, sy, 3);
}

/* coriander's Bayer decoding (GPL) */
void
BayerDownsample_uint16(const uint16_t * src, uint16_t * dest, int sx,
		       int sy, int type, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;
    int tmp;

    sx *= 2;
    sy *= 2;

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad Bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	for (i = 0; i < sy*sx; i += (sx<<1)) {
	    for (j = 0; j < sx; j += 2) {
		tmp =
		    ((src[i + j] + src[i + sx + j + 1]) >> 1);
		CLIP16(tmp, outG[((i >> 2) + (j >> 1)) * 3], bits);
		tmp = src[i + sx + j + 1];
		CLIP16(tmp, outR[((i >> 2) + (j >> 1)) * 3], bits);
		tmp = src[i + sx + j];
		CLIP16(tmp, outB[((i >> 2) + (j >> 1)) * 3], bits);
	    }
	}
	break;
    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	for (i = 0; i < sy*sx; i += (sx<<1)) {
	    for (j = 0; j < sx; j += 2) {
		tmp =
		    ((src[i + sx + j] + src[i + j + 1]) >> 1);
		CLIP16(tmp, outG[((i >> 2) + (j >> 1)) * 3], bits);
		tmp = src[i + sx + j + 1];
		CLIP16(tmp, outR[((i >> 2) + (j >> 1)) * 3], bits);
		tmp = src[i + j];
		CLIP16(tmp, outB[((i >> 2) + (j >> 1)) * 3], bits);
	    }
	}
	break;
    default:			//---------------------------------------------------------
	fprintf(stderr, "Bad Bayer pattern ID: %d\n", type);
	return;
	break;
    }

}

/* coriander's Bayer decoding (GPL) */
void
BayerSimple_uint16(const uint16_t * src, uint16_t * dest, int sx, int sy,
		   int type, int bits)
{
    uint16_t *outR, *outG, *outB;
    register int i, j;
    int tmp, base;

    // sx and sy should be even
    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:
    case COLOR_FILTER_FORMAT7_BGGR:
	outR = &dest[0];
	outG = &dest[1];
	outB = &dest[2];
	break;
    case COLOR_FILTER_FORMAT7_GBRG:
    case COLOR_FILTER_FORMAT7_RGGB:
	outR = &dest[2];
	outG = &dest[1];
	outB = &dest[0];
	break;
    default:
	outR = NULL;
	outG = NULL;
	outB = NULL;
	break;
    }

    switch (type) {
    case COLOR_FILTER_FORMAT7_GRBG:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_GBRG:
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + sx + 1]) >> 1);
		CLIP16(tmp, outG[base * 3], bits);
		tmp = src[base + 1];
		CLIP16(tmp, outR[base * 3], bits);
		tmp = src[base + sx];
		CLIP16(tmp, outB[base * 3], bits);
	    }
	}
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + 1] + src[base + sx]) >> 1);
		CLIP16(tmp, outG[(base) * 3], bits);
		tmp = src[base];
		CLIP16(tmp, outR[(base) * 3], bits);
		tmp = src[base + 1 + sx];
		CLIP16(tmp, outB[(base) * 3], bits);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + sx] + src[base + 1]) >> 1);
		CLIP16(tmp, outG[base * 3], bits);
		tmp = src[base + sx + 1];
		CLIP16(tmp, outR[base * 3], bits);
		tmp = src[base];
		CLIP16(tmp, outB[base * 3], bits);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + 1 + sx]) >> 1);
		CLIP16(tmp, outG[(base) * 3], bits);
		tmp = src[base + sx];
		CLIP16(tmp, outR[(base) * 3], bits);
		tmp = src[base + 1];
		CLIP16(tmp, outB[(base) * 3], bits);
	    }
	}
	break;
    case COLOR_FILTER_FORMAT7_BGGR:	//---------------------------------------------------------
    case COLOR_FILTER_FORMAT7_RGGB:
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + sx] + src[base + 1]) >> 1);
		CLIP16(tmp, outG[base * 3], bits);
		tmp = src[base + sx + 1];
		CLIP16(tmp, outR[base * 3], bits);
		tmp = src[base];
		CLIP16(tmp, outB[base * 3], bits);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 0; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + 1 + sx]) >> 1);
		CLIP16(tmp, outG[(base) * 3], bits);
		tmp = src[base + 1];
		CLIP16(tmp, outR[(base) * 3], bits);
		tmp = src[base + sx];
		CLIP16(tmp, outB[(base) * 3], bits);
	    }
	}
	for (i = 0; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base] + src[base + sx + 1]) >> 1);
		CLIP16(tmp, outG[base * 3], bits);
		tmp = src[base + sx];
		CLIP16(tmp, outR[base * 3], bits);
		tmp = src[base + 1];
		CLIP16(tmp, outB[base * 3], bits);
	    }
	}
	for (i = 1; i < sy - 1; i += 2) {
	    for (j = 1; j < sx - 1; j += 2) {
		base = i * sx + j;
		tmp = ((src[base + 1] + src[base + sx]) >> 1);
		CLIP16(tmp, outG[(base) * 3], bits);
		tmp = src[base];
		CLIP16(tmp, outR[(base) * 3], bits);
		tmp = src[base + 1 + sx];
		CLIP16(tmp, outB[(base) * 3], bits);
	    }
	}
	break;
    default:			//---------------------------------------------------------
	fprintf(stderr, "Bad bayer pattern ID: %d\n", type);
	return;
	break;
    }

    /* add black border */
    for (i = sx * (sy - 1) * 3; i < sx * sy * 3; i++) {
	dest[i] = 0;
    }
    for (i = (sx - 1) * 3; i < sx * sy * 3; i += (sx - 1) * 3) {
	dest[i++] = 0;
	dest[i++] = 0;
	dest[i++] = 0;
    }
}
