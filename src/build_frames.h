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


#ifndef __BUILDFRAMES_H__
#define __BUILDFRAMES_H__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif 

#include <gnome.h>
#include <libdc1394/dc1394_control.h>
#include "support.h"
#include "preferences.h"
#include "definitions.h"
#include "build_menus.h"
#include "tools.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "thread_iso.h"
#include "callbacks.h"

void
BuildPrefsGeneralFrame(void);

void
BuildCameraFrame(void);

void
BuildServiceFrame(void);

void
BuildLockFrame(void);

void
BuildTriggerFrame(void);

void
BuildPowerFrame(void);

void
BuildMemoryFrame(void);

void
BuildCaptureFrame(void);

void
BuildIsoFrame(void);

void
BuildGlobalIsoFrame(void);

void
BuildFormat7ModeFrame(void);

void
BuildCameraStatusFrame(void);

void
BuildTransferStatusFrame(void);

void
BuildPrefsSaveFrame(void);

void
BuildPrefsFtpFrame(void);

void
BuildPrefsDisplayFrame(void);

void
BuildPrefsV4lFrame(void);

void
BuildPrefsReceiveFrame(void);

void
BuildOptionFrame(void);

void
BuildBandwidthFrame(void);
#endif
