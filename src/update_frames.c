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
#include "update_frames.h"
#include "support.h"
#include "build_menus.h"
#include "definitions.h"
#include "tools.h"
#include "capture.h"
#include "build_ranges.h"
#include <string.h>
#include <libdc1394/dc1394_control.h>
#include "raw1394support.h"

extern GtkWidget *commander_window;
extern GtkWidget *status_window;
extern GtkWidget *capture_window;
extern dc1394_miscinfo *misc_info;
extern dc1394_feature_set *feature_set;
extern CtxtInfo ctxt;
extern dc1394_camerainfo *camera;
extern char* phy_speed_list[4];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern SelfIdPacket_t *selfid;
extern capture_info ci;

void
UpdateCameraFrame(void)
{
  // should reprobe the bus for new cameras here??
}

void
UpdateLockFrame(void)
{
  // nothing to update yet
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
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode >> 1)); // 2 or 3 means 2nd bit=1 => >>1==1
  gtk_widget_set_sensitive(lookup_widget(commander_window, "label16"),
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode >> 1)); // 2 or 3 means 2nd bit=1 => >>1==1

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
UpdateCaptureFrame(void)
{
  if (misc_info->is_iso_on>0)
    {
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( capture_window, "capture_start")), TRUE); // added by DDouxchamps
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( capture_window, "capture_single")), TRUE);//
    }
}

void
UpdateIsoFrame(void)
{

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
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_vendor_status"),
		       ctxt.vendor_ctxt, ctxt.vendor_id);
  ctxt.vendor_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(status_window,"camera_vendor_status"),
				    ctxt.vendor_ctxt, temp);

  // camera model:
  sprintf(temp," %s",camera->model);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_model_status"),
		       ctxt.model_ctxt, ctxt.model_id);
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(status_window,"camera_model_status"),
				    ctxt.model_ctxt, temp);

  // camera node:
  sprintf(temp," %d",camera->id);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_node_status"),
		       ctxt.node_ctxt, ctxt.node_id);
  ctxt.node_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(status_window,"camera_node_status"),
				   ctxt.node_ctxt, temp);

  // camera handle:
  sprintf(temp," 0x%x",(unsigned int)camera->handle);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_handle_status"),
		       ctxt.handle_ctxt, ctxt.handle_id);
  ctxt.handle_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_handle_status"),
				    ctxt.handle_ctxt, temp);

  // camera GUID:
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_guid_status"),
		       ctxt.guid_ctxt, ctxt.guid_id);
  ctxt.guid_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_guid_status"),
				  ctxt.guid_ctxt, " <Future Feature>");

  // camera maximal PHY speed:
  sprintf(temp," %s",phy_speed_list[selfid->packetZero.phySpeed]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_maxiso_status"),
		       ctxt.max_iso_ctxt, ctxt.max_iso_id);
  ctxt.max_iso_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_maxiso_status"),
				     ctxt.max_iso_ctxt, temp);

  // camera maximal PHY delay:
  sprintf(temp," %s",phy_delay_list[selfid->packetZero.phyDelay]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_delay_status"),
		       ctxt.delay_ctxt, ctxt.delay_id);
  ctxt.delay_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_delay_status"),
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
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_dc_status"),
		       ctxt.dc_ctxt, ctxt.dc_id);
  ctxt.dc_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_dc_status"),
				     ctxt.dc_ctxt, temp);

  // power class:
  sprintf(temp," %s",power_class_list[selfid->packetZero.powerClass]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(status_window,"camera_pwclass_status"),
		       ctxt.pwclass_ctxt, ctxt.pwclass_id);
  ctxt.pwclass_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(status_window,"camera_pwclass_status"),
				     ctxt.pwclass_ctxt,temp);

}


void
UpdateTransferStatusFrame(void)
{
  char temp[256];

  sprintf(temp," %d",misc_info->iso_channel);

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(status_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  sprintf(temp," %d",misc_info->iso_speed);

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(status_window,"iso_speed_status"), ctxt.iso_speed_ctxt, ctxt.iso_speed_id);
  ctxt.iso_speed_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_speed_status"), ctxt.iso_speed_ctxt, " <Future Feature>");

  if (misc_info->is_iso_on>0)
    sprintf(temp," Transmitting...");
  else
    sprintf(temp," No ISO transmission");

  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(status_window,"iso_status_status"), ctxt.iso_status_ctxt, ctxt.iso_status_id);
  ctxt.iso_status_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_status_status"), ctxt.iso_status_ctxt, temp);

}

void
UpdateFTPFrame(void)
{

  GtkWidget *window = gtk_widget_get_toplevel(lookup_widget(capture_window,"checkbutton_capture_ftp"));

  ci.ftp_enable = gtk_toggle_button_get_active((GtkToggleButton *)(lookup_widget(window,"checkbutton_capture_ftp")));

  gtk_widget_set_sensitive( lookup_widget(window, "entry_capture_ftp_address"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "entry_capture_ftp_path"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "entry_capture_ftp_user"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "entry_capture_ftp_passwd"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "label_capture_ftp_address"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "label_capture_ftp_path"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "label_capture_ftp_user"), ci.ftp_enable);
  gtk_widget_set_sensitive( lookup_widget(window, "label_capture_ftp_passwd"), ci.ftp_enable);
}
