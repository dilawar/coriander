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

#ifndef __BAYER_H__
#define __BAYER_H__

typedef enum
{
  NO_BAYER_DECODING=0,
  BAYER_DECODING_NEAREST,
  BAYER_DECODING_EDGE_SENSE,
  BAYER_DECODING_DOWNSAMPLE,
  BAYER_DECODING_SIMPLE,
  BAYER_DECODING_BILINEAR,
  BAYER_DECODING_HQLINEAR
} bayer_decoding_t;

void
BayerNearestNeighbor(const unsigned char *bayer, unsigned char *dst,
		     int sx, int sy, int code);

void
ClearBorders(unsigned char *dest, int sx, int sy, int w);

void
BayerBilinear(const unsigned char *bayer, unsigned char *dst, int sx,
	      int sy, int code);

void
BayerHQLinear(const unsigned char *bayer, unsigned char *dst, int sx,
	      int sy, int code);

void
BayerEdgeSense(const unsigned char *src, unsigned char *dest, int sx,
	       int sy, int type);

void
BayerDownsample(const unsigned char *src, unsigned char *dest, int sx,
		int sy, int type);

void
BayerSimple(const unsigned char *src, unsigned char *dest, int sx, int sy,
	    int type);

void
BayerNearestNeighbor_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
			    int sy, int code, int bits);

void
ClearBorders_uint16(uint16_t * dest, int sx, int sy, int w);

void
BayerBilinear_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
		     int sy, int code, int bits);

void
BayerHQLinear_uint16(const uint16_t * bayer, uint16_t * dst, int sx,
		     int sy, int code, int bits);

void
BayerEdgeSense_uint16(const uint16_t * src, uint16_t * dest, int sx,
		      int sy, int type, int bits);

void
BayerDownsample_uint16(const uint16_t * src, uint16_t * dest, int sx,
		       int sy, int type, int bits);

void
BayerSimple_uint16(const uint16_t * src, uint16_t * dest, int sx, int sy,
		   int type, int bits);


#endif

















