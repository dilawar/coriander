/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "support.h"
#include "thread_base.h"
#include "raw1394support.h"
#include "topology.h"
#include "update_windows.h"
#include "build_windows.h"
#include "update_frames.h"
#include "update_ranges.h"
#include "thread_iso.h"
#include "thread_ftp.h"
#include "thread_save.h"

/*
typedef struct _whitebaldata
{
  int x;
  int y;
  chain_t *service;
  pthread_t thread;

} whitebal_data_t;
*/

void
GetFormat7Capabilities(raw1394handle_t handle, nodeid_t node, Format7Info *info);

void
ChangeModeAndFormat         (GtkMenuItem     *menuitem,
			     gpointer         user_data);
void
IsoFlowCheck(int *state); 

void
IsoFlowResume(int *state);

void
GetContextStatus(void);

void
GrabSelfIds(raw1394handle_t* handles, int portmax);

void
SetChannels(void);

void
MainError(const char *string);

void
MainStatus(const char *string);

void
MessageBox(gchar *message);

void
SetScaleSensitivity(GtkWidget* widget, int feature, dc1394bool_t sense);

/*
void
GetRGBPix(int px, int py, chain_t *service, int* R, int* G, int* B);

void*
AutoWhiteBalance(void* arg);
*/
void
SetAbsoluteControl(int feature, int power);

dc1394bool_t
BuildAbsControl(int feature);

void
SetAbsValue(int feature);

void
GetAbsValue(int feature);

void
StopFPSDisplay(void);

void
ResumeFPSDisplay(void);

int
bus_reset_handler(raw1394handle_t handle, unsigned int generation);

int
main_timeout_handler(gpointer* port_num);

#endif

