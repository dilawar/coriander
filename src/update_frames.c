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
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "update_frames.h"
#include "support.h"
#include "build_menus.h"
#include "definitions.h"
#include "preferences.h"
#include "tools.h"
#include "thread_iso.h"
#include "thread_display.h" 
#include "thread_save.h"
#include "thread_ftp.h"
#include "thread_base.h"
#include "build_ranges.h"
#include <libdc1394/dc1394_control.h>
#include "raw1394support.h"

extern GtkWidget *commander_window;
extern GtkWidget *preferences_window;
extern dc1394_miscinfo *misc_info;
extern dc1394_feature_set *feature_set;
extern CtxtInfo ctxt;
extern dc1394_camerainfo *camera;
extern char* phy_speed_list[4];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern SelfIdPacket_t *selfid;
extern PrefsInfo preferences; 
extern int silent_ui_update;
extern int current_camera;

void
UpdatePrefsUpdateFrame(void)
{
  // nothing yet. should update ranges
}

void
UpdatePrefsDisplayFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_display_period"),
			    preferences.display_period);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_display_method_menu")),
			      preferences.display_method2index[preferences.display_method]);
}

void
UpdatePrefsReceiveFrame(void)
{
  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_receive_method_menu")),
			      preferences.receive_method2index[preferences.receive_method]);

}

void
UpdatePrefsSaveFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_save_period"), preferences.save_period);
  // scratch
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

  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_save_filename")),
		     preferences.save_filename);
}


void
UpdatePrefsFtpFrame(void)
{
#ifdef HAVE_FTPLIB
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_ftp_period"), preferences.ftp_period);
  // scratch
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
  // file,... names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_filename")),
		     preferences.ftp_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_address")),
		     preferences.ftp_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_password")),
		     preferences.ftp_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_path")),
		     preferences.ftp_path);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_user")),
		     preferences.ftp_user);

#else

  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_scratch_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_server_frame"),FALSE);

#endif

}

void
UpdatePrefsRealFrame(void)
{

#ifdef HAVE_REALLIB
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_server_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_infos_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_stream_frame"),TRUE);

  // names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_filename")),
		     preferences.real_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_address")),
		     preferences.real_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_password")),
		     preferences.real_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_user")),
		     preferences.real_user);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_title")),
		     preferences.real_title);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_author")),
		     preferences.real_author);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_copyright")),
		     preferences.real_copyright);

  // port
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window, "prefs_real_port"),
			    preferences.real_port);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_real_audience")),
			      preferences.real_audience);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_real_quality")),
			      preferences.real_quality);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_real_compatibility")),
			      preferences.real_compatibility);

  // recordable?
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_record_yes")),
			       preferences.real_recordable);

#else
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_server_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_infos_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_stream_frame"),FALSE);
#endif

}


void
UpdateCameraFrame(void)
{
  // should reprobe the bus for new cameras here??
}

void
UpdateTriggerFrame(void)
{
  // always set the trigger frame on (because it contains the fps menu):
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_frame"),TRUE);

  /*printf("is_on: %d available: %d\n",
	 feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on,
	 feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available);*/
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_external"),
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"fps_menu"),
			   !(feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on));
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_mode"),
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on && 
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_polarity"),
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available &&
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on &&
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].polarity_capable);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_count"),
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode >> 1));
  // 2 or 3 means 2nd bit=1 => >>1==1
  gtk_widget_set_sensitive(lookup_widget(commander_window, "label16"),
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode >> 1));
  // 2 or 3 means 2nd bit=1 => >>1==1

}

void
UpdatePowerFrame(void)
{
  // nothing to update

}

void
UpdateMemoryFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(commander_window,"memory_channel"),TRUE);

  // save not activated by default (it is not avail. for factory defaults channel):
  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(commander_window,"save_mem")),
			   ((misc_info->mem_channel_number>0)&&(misc_info->save_channel>0)));

  // load always present, so we can activate it:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"memory_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"load_mem"),TRUE);
}


void
UpdateIsoFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_start"),!misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_restart"),misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_stop"),misc_info->is_iso_on);
}

void
UpdateFormat7ModeFrame(void)
{
  BuildFormat7ColorMenu();
  BuildFormat7ModeMenu();
  BuildFormat7Ranges();
}

void
UpdateCameraStatusFrame(void)
{
  char temp[256];
  int err;
  quadlet_t sw_version;

  // vendor:
  sprintf(temp," %s",camera->vendor);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_vendor_status"),
		       ctxt.vendor_ctxt, ctxt.vendor_id);
  ctxt.vendor_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"camera_vendor_status"),
				    ctxt.vendor_ctxt, temp);

  // camera model:
  sprintf(temp," %s",camera->model);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_model_status"),
		       ctxt.model_ctxt, ctxt.model_id);
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(commander_window,"camera_model_status"),
				    ctxt.model_ctxt, temp);

  // camera node:
  sprintf(temp," %d",camera->id);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_node_status"),
		       ctxt.node_ctxt, ctxt.node_id);
  ctxt.node_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(commander_window,"camera_node_status"),
				   ctxt.node_ctxt, temp);

  // camera handle:
  sprintf(temp," 0x%x",(unsigned int)camera->handle);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_handle_status"),
		       ctxt.handle_ctxt, ctxt.handle_id);
  ctxt.handle_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_handle_status"),
				    ctxt.handle_ctxt, temp);

  // camera GUID:
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_guid_status"),
		       ctxt.guid_ctxt, ctxt.guid_id);
  ctxt.guid_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_guid_status"),
				  ctxt.guid_ctxt, " <Future Feature>");

  // camera maximal PHY speed:
  sprintf(temp," %s",phy_speed_list[selfid->packetZero.phySpeed]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_maxiso_status"),
		       ctxt.max_iso_ctxt, ctxt.max_iso_id);
  ctxt.max_iso_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_maxiso_status"),
				     ctxt.max_iso_ctxt, temp);

  // camera maximal PHY delay:
  sprintf(temp," %s",phy_delay_list[selfid->packetZero.phyDelay]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_delay_status"),
		       ctxt.delay_ctxt, ctxt.delay_id);
  ctxt.delay_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_delay_status"),
				     ctxt.delay_ctxt, temp);

  // IIDC software revision:
  err=dc1394_get_sw_version(camera->handle, camera->id, &sw_version);
  if (!err)
    {
      MainError("Could not get the IIDC software revision");
      sw_version=0x000000;
    }
  switch (sw_version)
    {
    case 0x000100: sprintf(temp," 1.04");break;
    case 0x000101: sprintf(temp," 1.20");break;
    case 0x000102: sprintf(temp," 1.30");break;
    default: sprintf(temp," Unknown IIDC Specs version");
    }
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_dc_status"),
		       ctxt.dc_ctxt, ctxt.dc_id);
  ctxt.dc_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_dc_status"),
				     ctxt.dc_ctxt, temp);

  // power class:
  sprintf(temp," %s",power_class_list[selfid->packetZero.powerClass]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_pwclass_status"),
		       ctxt.pwclass_ctxt, ctxt.pwclass_id);
  ctxt.pwclass_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_pwclass_status"),
				     ctxt.pwclass_ctxt,temp);

}


void
UpdateTransferStatusFrame(void)
{
  char temp[256];
  int err;
  sprintf(temp," %d",misc_info->iso_channel);

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  sprintf(temp," %d",misc_info->iso_speed);

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_speed_status"), ctxt.iso_speed_ctxt, ctxt.iso_speed_id);
  ctxt.iso_speed_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_speed_status"), ctxt.iso_speed_ctxt, " <Future Feature>");

  // we perform an update of the ISO local info here. Just to avoid possible incoherencies.
  err=dc1394_get_iso_status(camera->handle, camera->id, &misc_info->is_iso_on);
  if (misc_info->is_iso_on>0)
    sprintf(temp," Transmitting...");
  else
    sprintf(temp," No ISO transmission");

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_status_status"), ctxt.iso_status_ctxt, ctxt.iso_status_id);
  ctxt.iso_status_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_status_status"), ctxt.iso_status_ctxt, temp);

  err=dc1394_get_iso_channel_and_speed(camera->handle, camera->id, &misc_info->iso_channel, &misc_info->iso_speed);
  sprintf(temp," %d",misc_info->iso_channel);

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

}


void
UpdateServicesFrame(void)
{
  silent_ui_update=1;

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")),
			       GetService(SERVICE_ISO,current_camera)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_display")),
			       GetService(SERVICE_DISPLAY,current_camera)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_save")),
			       GetService(SERVICE_SAVE,current_camera)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_ftp")),
			       GetService(SERVICE_FTP,current_camera)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_real")),
			       GetService(SERVICE_FTP,current_camera)!=NULL);
  silent_ui_update=0;

}
