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

#include "coriander.h"

void
UpdateFormat7Window(void)
{
  UpdateFormat7Ranges();
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
  //  UpdateFormatMenu();
  UpdateIsoFrame();
  UpdateOptionFrame();
  UpdateServiceFrame();
}

void
UpdateFeatureWindow(void)
{
  int i;

  //char stemp[256];

  for (i=DC1394_FEATURE_MIN;i<=DC1394_FEATURE_MAX;i++) {
    if ((camera->feature_set.feature[i-DC1394_FEATURE_MIN].available>0)&&
	(i!=DC1394_FEATURE_TRIGGER)) {
      UpdateRange(i);
      /*
	fprintf(stderr,"%d  : %d %d %d %d %d\n",i,
	camera->feature_set.feature[i-DC1394_FEATURE_MIN].on_off_capable,
	camera->feature_set.feature[i-DC1394_FEATURE_MIN].manual_capable,
	camera->feature_set.feature[i-DC1394_FEATURE_MIN].auto_capable,
	camera->feature_set.feature[i-DC1394_FEATURE_MIN].one_push_capable,
	camera->feature_set.feature[i-DC1394_FEATURE_MIN].absolute_capable);
      */
      // if there is no control mode available for the feature, disable it.
      if (!((camera->feature_set.feature[i-DC1394_FEATURE_MIN].on_off_capable  || // disable feature if there is no way to control it
	     (camera->feature_set.feature[i-DC1394_FEATURE_MIN].modes.num>0)  ||
	     camera->feature_set.feature[i-DC1394_FEATURE_MIN].absolute_capable ) &&
	    (camera->feature_set.feature[i-DC1394_FEATURE_MIN].on_off_capable ||  // disable features that are OFF and not ON-settable
	     camera->feature_set.feature[i-DC1394_FEATURE_MIN].is_on ||
	     ((camera->camera_info->guid>>40)!=0xb09d)) // ptgrey only
	    )) {
	  // FIXME: should set each of the feature components off individually
	  //sprintf(stemp,"feature_%d_frame",i);
	  //gtk_widget_set_sensitive(lookup_widget(main_window, stemp), 0);
	
      }
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
