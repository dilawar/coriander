/*
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
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

#ifndef __UPDATEFRAMES_H__
#define __UPDATEFRAMES_H__

void
UpdatePrefsUpdateFrame(void);

void
UpdatePrefsDisplayFrame(void);

void
UpdatePrefsReceiveFrame(void);

void
UpdatePrefsRealFrame(void); 

void
UpdateCameraFrame(void);

void
UpdateTriggerFrame(void);

void
UpdatePowerFrame(void);

void
UpdateMemoryFrame(void);

void
UpdateCaptureFrame(void);

void
UpdateIsoFrame(void);

void
UpdateFormat7ModeFrame(void);

void
UpdateCameraStatusFrame(void);

void
UpdateTransferStatusFrame(void);

void
UpdatePrefsSaveFrame(void);

void
UpdatePrefsFtpFrame(void);

void
UpdateServicesFrame(void);

#endif
