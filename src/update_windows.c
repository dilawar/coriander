/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#include "update_windows.h"

extern camera_t* camera;
extern unsigned int format7_tab_presence;

void
UpdateFormat7Window(void)
{
  UpdateFormat7Ranges();
  UpdateFormat7BppRange();
  BuildFormat7ColorMenu();
  UpdateFormat7InfoFrame();
}

void
UpdatePreferencesWindow(void)
{
  UpdatePrefsReceiveFrame();    
  UpdatePrefsDisplayFrame();
  UpdatePrefsGeneralFrame();    
  UpdatePrefsSaveFrame();
  UpdatePrefsFtpFrame();
  UpdatePrefsV4lFrame();
}

void
UpdateCommanderWindow(void)
{
  UpdateCameraFrame();
  UpdateTriggerFrame();
  UpdatePowerFrame();
  UpdateMemoryFrame();
  UpdateFormatMenu();
  UpdateIsoFrame();
  UpdateOptionFrame();
  UpdateServiceFrame();
}

void
UpdateFeatureWindow(void)
{
  int i;

  for (i=FEATURE_MIN;i<=FEATURE_MAX;i++) {
    if ((camera->feature_set.feature[i-FEATURE_MIN].available>0)&&
	(i!=FEATURE_TRIGGER)) {
      UpdateRange(i);
    }
  }
}

void
UpdateStatusWindow(void)
{
  UpdateCameraStatusFrame();
  UpdateTransferStatusFrame();
  UpdateBandwidthFrame();
}


void
UpdateAllWindows(void)
{
  UpdatePreferencesWindow();
  UpdateFeatureWindow();
  UpdateStatusWindow();
  UpdateCommanderWindow();
  if ((camera->format7_info.edit_mode!=-1)&&(format7_tab_presence==1))
    UpdateFormat7Window();
}
