/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
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

#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <libdc1394/dc1394_control.h>
#define  LOOP_RETRIES          20     // # of tries before timeout,
                                      // thus timeout is LOOP_RETRIES*LOOP_SLEEP
#define  LOOP_SLEEP        500000     // .5 second
#define  BU                  1000     // definitions for distinguishing the BU and RV
#define  RV                  2000     // scales of the FEATURE_WHITE_BALANCE feature
#define  NO                 FALSE
#define  YES                 TRUE


enum 
{
    FORMAT7_SIZE_X = 0,
    FORMAT7_SIZE_Y,
    FORMAT7_POS_X,
    FORMAT7_POS_Y
};

typedef struct _Format7ModeInfo
{

  dc1394bool_t present;

  unsigned int size_x;
  unsigned int size_y;
  unsigned int max_size_x;
  unsigned int max_size_y;

  unsigned int pos_x;
  unsigned int pos_y;

  unsigned int step_x;
  unsigned int step_y;

  unsigned int color_coding_id;
  quadlet_t color_coding;

  unsigned int pixnum;

  unsigned int bpp; // bpp is byte_per_packet
  unsigned int min_bpp;
  unsigned int max_bpp;

  unsigned int total_bytes;

} Format7ModeInfo;

typedef struct _Format7Info
{
  Format7ModeInfo mode[NUM_MODE_FORMAT7];
  int edit_mode;

} Format7Info;

typedef struct _UIInfo
{
  int overlay_power;
  int test_pattern;
  int all_auto;
  int all_man;
  int all_lock;
} UIInfo;

typedef struct _StatusInfo
{
  char name[256];
  quadlet_t guid;
  
} StatusInfo;

typedef struct _CtxtInfo
{
  int model_ctxt;
  int node_ctxt;
  int guid_ctxt;
  int handle_ctxt;
  int vendor_ctxt;
  int max_iso_ctxt;
  int delay_ctxt;
  int dc_ctxt;
  int pwclass_ctxt;

  int iso_speed_ctxt;
  int iso_channel_ctxt;
  int iso_status_ctxt;

  int main_ctxt;

  int max_iso_id;
  int model_id;
  int node_id;
  int guid_id;
  int handle_id;
  int vendor_id;
  int delay_id;
  int dc_id;
  int pwclass_id;

  int iso_speed_id;
  int iso_channel_id;
  int iso_status_id;

  int main_id;

} CtxtInfo;

typedef struct _PrefsInfo
{
  float op_timeout;
  int auto_update;
  float auto_update_frequency;
  int display_method;
  int receive_method;

} PrefsInfo;

#endif

