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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <libdc1394/dc1394_control.h>
#include <libraw1394/raw1394.h>

void
GetFormat7Capabilities(raw1394handle_t handle, nodeid_t node, Format7Info *info);

void
ChangeModeAndFormat(int mode, int format);

void
IsoFlowCheck(void);

void
IsoFlowResume(void);

void
GetContextStatus(void);

void
GrabSelfIds(raw1394handle_t handle);

void
SelectCamera(int i);

void
MainError(const char *string);

void
UpdateIdler(void);

#endif
