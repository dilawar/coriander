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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <libdc1394/dc1394_control.h>
#include "definitions.h"
#include "raw1394support.h"
#include "tools.h"

void
GetCameraNodes(BusInfo_t* businfo);

void
GetCamerasInfo(BusInfo_t* businfo);

camera_t*
NewCamera(void);

void
GetCameraData(raw1394handle_t handle, nodeid_t node, camera_t* camera);

void
AppendCamera(camera_t* camera);

void
SetCurrentCamera(u_int64_t guid);

void
RemoveCamera(u_int64_t guid);

void
FreeCamera(camera_t* camera);

#endif