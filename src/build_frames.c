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
#include "thread_ftp.h"
#include "thread_save.h"
#include "preferences.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *commander_window;
extern GtkWidget *preferences_window;
extern dc1394_feature_set *feature_set;
extern dc1394_camerainfo *camera;
extern dc1394_miscinfo *misc_info;
extern PrefsInfo preferences;

void
BuildCameraFrame(void)
{
  BuildCameraMenu();
}

void
BuildServiceFrame(void)
{
#ifdef HAVE_FTPLIB
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_ftp"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_ftp"),FALSE);
#endif
#ifdef HAVE_REALLIB
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_real"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_real"),FALSE);
#endif
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
BuildIsoFrame(void)
{
  int err;
  // TODO: only if ISO capable
  err=dc1394_get_iso_status(camera->handle,camera->id,&misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_start"),!misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_restart"),misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_stop"),misc_info->is_iso_on);

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


void
BuildPrefsSaveFrame(void)
{
  GtkAdjustment *adjustment;

  switch(preferences.save_mode)
    {
    case SAVE_MODE_IMMEDIATE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_afap"), TRUE);
      break;
    case SAVE_MODE_PERIODIC:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_every"), TRUE);
      break;
    }

  switch(preferences.save_scratch)
    {
    case SAVE_SCRATCH_OVERWRITE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_scratch"),TRUE);
      break;
    case SAVE_SCRATCH_SEQUENTIAL:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_seq"),TRUE);
      break;
    }
  // set the  adjustment for spin buttons:
  adjustment=(GtkAdjustment*)gtk_adjustment_new(1,1,999999,1,100,0);
  gtk_spin_button_set_adjustment((GtkSpinButton*)lookup_widget(preferences_window,
							       "prefs_save_period"),adjustment);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_save_period"), preferences.save_period);

}

void
BuildPrefsFtpFrame(void)
{

  //fprintf(stderr,"Entering buildframe\n");
#ifdef HAVE_FTPLIB

  GtkAdjustment *adjustment;

  switch(preferences.ftp_mode)
    {
    case FTP_MODE_IMMEDIATE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_afap"), TRUE);
      break;
    case FTP_MODE_PERIODIC:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_every"), TRUE);
      break;
    }

  switch(preferences.ftp_scratch)
    {
    case FTP_SCRATCH_OVERWRITE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_scratch"),TRUE);
      break;
    case FTP_SCRATCH_SEQUENTIAL:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_seq"),TRUE);
      break;
    }
  // set the  adjustment for spin button:
  adjustment=(GtkAdjustment*)gtk_adjustment_new(1,1,999999,1,100,0);
  gtk_spin_button_set_adjustment((GtkSpinButton*)lookup_widget(preferences_window,
							       "prefs_ftp_period"),adjustment);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_ftp_period"), preferences.ftp_period);

#else

  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_scratch_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_server_frame"),FALSE);

#endif
  //fprintf(stderr,"Exiting buildframe\n");

}

void
BuildPrefsRealFrame(void)
{

  GtkAdjustment *adjustment;
  GtkMenuItem* menuitem;
  GtkMenu* menu;
  GtkOptionMenu* option_menu;
  int i;

  
  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_audience");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<8;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_audience_activate),
			  (int*)i);
    }

  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_quality");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<4;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_quality_activate),
			  (int*)i);
    }

  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_compatibility");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<2;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_compatibility_activate),
			  (int*)i);
    }

  //fprintf(stderr,"Exiting buildframe\n");

  adjustment=(GtkAdjustment*)gtk_adjustment_new(1,1,9999,1,10,0);
  gtk_spin_button_set_adjustment((GtkSpinButton*)lookup_widget(preferences_window,
							       "prefs_real_port"),adjustment);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_real_port"), preferences.real_port);

  // TODO: connect signals

}
