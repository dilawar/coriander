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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include "callbacks.h"
#include "support.h"
#include "definitions.h"
#include "build_menus.h"
#include "build_ranges.h"
#include "build_frames.h"
#include "update_frames.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *commander_window;
extern dc1394_feature_set *feature_set;
extern dc1394_camerainfo *camera;
extern GtkWidget *capture_window;

void
BuildCameraFrame(void)
{
  BuildCameraMenu();
}

void
BuildLockFrame(void)
{
  // nothing to build!
}


void
BuildTriggerFrame(void)
{
  GtkAdjustment *adjustment;

  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_frame"),TRUE);

  BuildTriggerModeMenu();
  BuildFpsMenu();
  
  // set the trigger_count value adjustment
  adjustment=(GtkAdjustment*)gtk_adjustment_new(0,1,(int)0xFFF,1,10,0);// max. number for trigger parameter is 12bit=FFFh
  gtk_spin_button_set_adjustment((GtkSpinButton*)lookup_widget(commander_window, "trigger_count"),adjustment);
 
  // TODO: connect signal
}


void
BuildPowerFrame(void)
{
  quadlet_t basic_funcs;
  int err;
  // these two functions are always present:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_reset"),TRUE);

  // activate if camera capable of power on/off:
  err=dc1394_query_basic_functionality(camera->handle,camera->id,&basic_funcs);
  if (!err) MainError("Could not query basic functionalities");

  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_on"),(basic_funcs & 0x1<<16));
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_off"),(basic_funcs & 0x1<<16));

}


void
BuildMemoryFrame(void)
{
  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"memory_frame"),TRUE);

  // activate the mem channel menu:
  BuildMemoryChannelMenu();

}

void
BuildCaptureFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(capture_window,"capture_frame"),TRUE);
}

void
BuildIsoFrame(void)
{
  // TODO: only if ISO capable
  gtk_widget_set_sensitive(lookup_widget(capture_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(capture_window,"iso_start"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(capture_window,"iso_restart"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(capture_window,"iso_stop"),TRUE);

}

void
BuildFormat7ModeFrame(void)
{

  BuildFormat7ModeMenu();
  BuildFormat7ColorMenu();
  BuildFormat7Ranges();
  
}

void
BuildCameraStatusFrame(void)
{ 
}


void
BuildTransferStatusFrame(void)
{
}



