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

extern UIInfo *uiinfo;
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
extern Format7Info *format7_info;
extern int silent_ui_update;
extern int current_camera;

void
UpdatePrefsGeneralFrame(void)
{
  // nothing yet. should update ranges
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(preferences_window,"prefs_update_power")),
			       preferences.auto_update);
}

void
UpdatePrefsDisplayFrame(void)
{
}

void
UpdatePrefsReceiveFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_video1394_device"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"label84"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_receive_dropframes"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
}

void
UpdatePrefsSaveFrame(void)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(preferences_window,"prefs_save_noconvert")),
			       preferences.save_scratch==SAVE_SCRATCH_SEQUENCE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_save_convert"),
			   preferences.save_scratch!=SAVE_SCRATCH_SEQUENCE);
}


void
UpdatePrefsFtpFrame(void)
{
}

void
UpdatePrefsRealFrame(void)
{
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

  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_external"),
			   feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"fps_menu"),
			   !(feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) &&
			   (misc_info->format != FORMAT_SCALABLE_IMAGE_SIZE));
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
			   ((feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_2)||
			     feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_3));

  gtk_widget_set_sensitive(lookup_widget(commander_window, "label16"),
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   ((feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_2)||
			     feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_3));
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
  char temp[STRING_SIZE];
  int err;
  quadlet_t sw_version;
  quadlet_t value[3];

  value[0]= camera->euid_64 & 0xffffffff;
  value[1]= (camera->euid_64 >>32) & 0x000000ff;
  value[2]= (camera->euid_64 >>40) & 0xfffff;

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
  sprintf(temp," 0x%06x-%02x%08x", value[2], value[1], value[0]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"camera_guid_status"),
		       ctxt.guid_ctxt, ctxt.guid_id);
  ctxt.guid_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(commander_window,"camera_guid_status"),
				  ctxt.guid_ctxt, temp);

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
  if (err<0)
    {
      MainError("Could not get the IIDC software revision");
      sw_version=0x000000;
    }
  switch (sw_version)
    {
    case 0x000100: sprintf(temp," 1.04");break;
    case 0x000101: sprintf(temp," 1.20");break;
    case 0x000102: sprintf(temp," 1.30");break;
    case 0x000114: sprintf(temp," Point Grey 114");break;
    default: sprintf(temp," Unknown IIDC Specs");
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

  // camera name: 
  //fprintf(stderr,"name: %s\n",preferences.camera_names[current_camera]);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window,"camera_name_text")),
  		     preferences.camera_names[current_camera]);

}


void
UpdateTransferStatusFrame(void)
{
  char temp[STRING_SIZE];
  int err;
  sprintf(temp," %d",misc_info->iso_channel);
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  sprintf(temp," %d",misc_info->iso_speed);
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"iso_speed_status"), ctxt.iso_speed_ctxt, ctxt.iso_speed_id);
  ctxt.iso_speed_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"iso_speed_status"), ctxt.iso_speed_ctxt, " N/A");

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

void
UpdateCursorFrame(int posx, int posy, int r, int g, int b, int y, int u, int v)
{
  char temp[STRING_SIZE];
  //fprintf(stderr,"Cursor Position: %d %d\n",posx,posy);

  fprintf(stderr,"");// Don't know why, but this seems necessary to make the cursor info work...
  // position: 
  sprintf(temp," %d",posx);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_x"), ctxt.cursor_x_ctxt, ctxt.cursor_x_id);
  ctxt.cursor_x_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_x"), ctxt.cursor_x_ctxt, temp);

  sprintf(temp," %d",posy);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_y"), ctxt.cursor_y_ctxt, ctxt.cursor_y_id);
  ctxt.cursor_y_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_y"), ctxt.cursor_y_ctxt, temp);
  
  // color:
  if (r>-255)
    {
      sprintf(temp," %d",r);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_r"),
			   ctxt.cursor_color_r_ctxt, ctxt.cursor_color_r_id);
      ctxt.cursor_color_r_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_r"),
						ctxt.cursor_color_r_ctxt, temp);
    }
  
  if (g>-255)
    {
      sprintf(temp," %d",g);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_g"),
			   ctxt.cursor_color_g_ctxt, ctxt.cursor_color_g_id);
      ctxt.cursor_color_g_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_g"),
						ctxt.cursor_color_g_ctxt, temp);
    }
  if (b>-255)
    {
      sprintf(temp," %d",b);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_b"),
			   ctxt.cursor_color_b_ctxt, ctxt.cursor_color_b_id);
      ctxt.cursor_color_b_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_b"),
						ctxt.cursor_color_b_ctxt, temp);
    }
  if (y>-255)
    {
      sprintf(temp," %d",y);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_y"),
			   ctxt.cursor_color_y_ctxt, ctxt.cursor_color_y_id);
      ctxt.cursor_color_y_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_y"),
						ctxt.cursor_color_y_ctxt, temp);
    }
  if (u>-255)
    {
      sprintf(temp," %d",u);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_u"),
			   ctxt.cursor_color_u_ctxt, ctxt.cursor_color_u_id);
      ctxt.cursor_color_u_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_u"),
						ctxt.cursor_color_u_ctxt, temp);
    }
  if (v>-255)
    {
      sprintf(temp," %d",v);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"cursor_color_v"),
			   ctxt.cursor_color_v_ctxt, ctxt.cursor_color_v_id);
      ctxt.cursor_color_v_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"cursor_color_v"),
						ctxt.cursor_color_v_ctxt, temp);
    }
}

void
UpdateOptionFrame(void)
{
  int cond16;
  int cond8;
  gtk_widget_set_sensitive(lookup_widget(commander_window,"pattern_menu"),
			   uiinfo->bayer!=NO_BAYER_DECODING);
  if (misc_info->format!=FORMAT_SCALABLE_IMAGE_SIZE)
    {
      cond8=((misc_info->mode==MODE_640x480_MONO)||
	     (misc_info->mode==MODE_800x600_MONO)||
	     (misc_info->mode==MODE_1024x768_MONO)||
	     (misc_info->mode==MODE_1280x960_MONO)||
	     (misc_info->mode==MODE_1600x1200_MONO));
      cond16=((misc_info->mode==MODE_640x480_MONO16)||
	      (misc_info->mode==MODE_800x600_MONO16)||
	      (misc_info->mode==MODE_1024x768_MONO16)||
	      (misc_info->mode==MODE_1280x960_MONO16)||
	      (misc_info->mode==MODE_1600x1200_MONO16));
    }
  else
    {
      cond16=(format7_info->mode[misc_info->mode].color_coding_id==COLOR_FORMAT7_MONO16);
      cond8=(format7_info->mode[misc_info->mode].color_coding_id==COLOR_FORMAT7_MONO8);
    }
  gtk_widget_set_sensitive(lookup_widget(commander_window,"pattern_menu"),(cond8||cond16));
  gtk_widget_set_sensitive(lookup_widget(commander_window,"bayer_menu"),(cond8||cond16));
  gtk_widget_set_sensitive(lookup_widget(commander_window,"stereo_button"),cond16);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"mono16_bpp"),cond16&&
			   (uiinfo->stereo==NO_STEREO_DECODING)&&(uiinfo->bayer==NO_BAYER_DECODING));
  
}
