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
#include "capture.h"
#include "tools.h"
#include "build_menus.h"
#include "update_frames.h"
#include "raw1394support.h"
#include "topology.h"
#include <libdc1394/dc1394_control.h>
#include <string.h>

extern GtkWidget *commander_window;
extern GtkWidget *porthole_window;
extern GtkWidget *color_window;
extern GtkWidget *status_window;
extern GtkWidget *aperture_window;
extern GtkWidget *configure_window;
extern GtkWidget *capture_window;
extern GtkWidget *temperature_window;
extern dc1394_camerainfo *camera;
extern dc1394_camerainfo *cameras;
extern dc1394_miscinfo *misc_info;
extern dc1394_miscinfo *misc_infos;
extern dc1394_feature_set *feature_set;
extern dc1394_feature_set *feature_sets;
extern Format7Info *format7_info;
extern Format7Info *format7_infos;
extern dc1394_cameracapture *capture;
extern dc1394_cameracapture *captures;
extern UIInfo *uiinfos;
extern UIInfo *uiinfo;
extern SelfIdPacket_t *selfid;
extern SelfIdPacket_t *selfids;
extern char* feature_op_list[NUM_FEATURES];
extern char* feature_list[NUM_FEATURES];
extern char* feature_frame_list[NUM_FEATURES];
extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_man_list[NUM_FEATURES];
extern char* feature_auto_list[NUM_FEATURES];
extern char* feature_power_list[NUM_FEATURES];
extern char* trigger_mode_list[4];
extern char* channel_num_list[16];
extern char* phy_speed_list[4];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern int camera_num;
extern int current_camera;
extern guint gIdleID;
extern porthole_info pi;
extern CtxtInfo ctxt;



void
GetFormat7Capabilities(raw1394handle_t handle, nodeid_t node, Format7Info *info)
{
  int i, f, err;
  quadlet_t value;
  
  err=dc1394_query_supported_modes(handle, node, FORMAT_SCALABLE_IMAGE_SIZE, &value);
  if (!err) MainError("Could not query Format7 supported modes");
  else
    {
      for (i=0,f=MODE_FORMAT7_MIN;f<MODE_FORMAT7_MAX;f++,i++)
	{
	  info->mode[i].present= (value & (0x1<<(31-i)) );
	  if (info->mode[i].present) // check for mode presence before query
	    {
	      err=1;
	      err*=dc1394_query_format7_max_image_size(handle,node,f,&info->mode[i].max_size_x,&info->mode[i].max_size_y);
	      err*=dc1394_query_format7_unit_size(handle,node,f,&info->mode[i].step_x,&info->mode[i].step_y);
	      err*=dc1394_query_format7_image_position(handle,node,f,&info->mode[i].pos_x,&info->mode[i].pos_y);
	      err*=dc1394_query_format7_image_size(handle,node,f,&info->mode[i].size_x,&info->mode[i].size_y);
	      
	      err*=dc1394_query_format7_pixel_number(handle,node,f,&info->mode[i].pixnum);
	      err*=dc1394_query_format7_byte_per_packet(handle,node,f,&info->mode[i].bpp);
	      err*=dc1394_query_format7_packet_para(handle,node,f,&info->mode[i].min_bpp,&info->mode[i].max_bpp);
	      err*=dc1394_query_format7_total_bytes(handle,node,f,&info->mode[i].total_bytes);
	      
	      // TODO: get color coding id
	      err*=dc1394_query_format7_color_coding(handle,node,f,&info->mode[i].color_coding);
	      if (!err) MainError("Got a problem querying format7 capabilitie");
	    }
	}
    }
  info->edit_mode = MODE_FORMAT7_MIN;

}

void
ChangeModeAndFormat(int mode, int format)
{
  int err;
  IsoFlowCheck();
  err=dc1394_set_video_format(camera->handle,camera->id,format);
  if (!err) MainError("Could not set video format");
  else misc_info->format=format;
  err=dc1394_set_video_mode(camera->handle,camera->id,mode);
  if (!err) MainError("Could not set video mode");
  else misc_info->mode=mode;
  BuildFpsMenu();
  UpdateTriggerFrame();
  IsoFlowResume();
}

void IsoFlowCheck()
{ 
  int err;

  if ( (pi.is_open = (pi.handle != NULL)) )
    {
      gtk_idle_remove(gIdleID);
      IsoStopThread();
    }
  err=dc1394_get_iso_channel_and_speed(camera->handle, camera->id, &misc_info->iso_channel, &misc_info->iso_speed);
  if (!err) MainError("Could not get ISO channel and speed");
  else
    {
      err=dc1394_get_iso_status(camera->handle, camera->id, &misc_info->is_iso_on);
      if (!err) MainError("Could not get ISO status");
      else
	{
	  if (misc_info->is_iso_on)
	    {
	      err=dc1394_stop_iso_transmission(camera->handle, camera->id);// if not done, restarting is no more possible
	      if (!err) MainError("Could not stop ISO transmission");
	    }
	}
    }
}

void IsoFlowResume()
{
  int err;
  GtkWidget *window = lookup_widget(porthole_window, "camera_scope");

  if (misc_info->is_iso_on)// restart if it was 'on' before the changes
    {
      err=dc1394_set_iso_channel_and_speed(camera->handle, camera->id, misc_info->iso_channel, misc_info->iso_speed);
      if (!err) MainError("Could not set ISO channel and speed");
      else 
	{
          err=dc1394_start_iso_transmission(camera->handle, camera->id);
	  if (!err) MainError("Could not start ISO transmission");
	  else
	    {
	      if (pi.is_open && (IsoStartThread(window)>0) )
		gIdleID = gtk_idle_add( porthole_idler, NULL);  
	    }
	}
    }
}

void GetContextStatus()
{
  ctxt.model_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_model_status"),"");
  ctxt.vendor_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_vendor_status"),"");
  ctxt.handle_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_handle_status"),"");
  ctxt.node_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_node_status"),"");
  ctxt.guid_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_guid_status"),"");
  ctxt.max_iso_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_maxiso_status"),"");
  ctxt.delay_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_delay_status"),"");
  ctxt.dc_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_dc_status"),"");
  ctxt.pwclass_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"camera_pwclass_status"),"");

  ctxt.iso_channel_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"iso_channel_status"),"");
  ctxt.iso_speed_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"iso_speed_status"),"");
  ctxt.iso_status_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(status_window,"iso_status_status"),"");

  ctxt.main_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(commander_window,"main_status"),"");

  // init message ids.
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_model_status"), ctxt.model_ctxt, "");
  ctxt.vendor_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_vendor_status"), ctxt.vendor_ctxt, "");
  ctxt.handle_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_handle_status"), ctxt.handle_ctxt, "");
  ctxt.node_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_node_status"), ctxt.node_ctxt, "");
  ctxt.guid_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_guid_status"), ctxt.guid_ctxt, "");
  ctxt.max_iso_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, "");
  ctxt.delay_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_delay_status"), ctxt.delay_ctxt, "");
  ctxt.dc_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_dc_status"), ctxt.dc_ctxt, "");
  ctxt.pwclass_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"camera_pwclass_status"), ctxt.pwclass_ctxt, "");

  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_channel_status"), ctxt.iso_channel_ctxt, "");
  ctxt.iso_speed_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_speed_status"), ctxt.iso_speed_ctxt, "");
  ctxt.iso_status_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(status_window,"iso_status_status"), ctxt.iso_status_ctxt, "");

  ctxt.main_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"main_status"), ctxt.main_ctxt, "");

  // note: these empty messages will be replaced after the execution of update_frame for status window
}

void GrabSelfIds(raw1394handle_t handle)
{

  RAW1394topologyMap *topomap;
  SelfIdPacket_t packet;
  unsigned int* pselfid_int;
  int i, j;

  // get and decode SelfIds.
  topomap=raw1394GetTopologyMap(handle);

  for (i=0;i<topomap->selfIdCount;i++)
    {
      pselfid_int = (unsigned int *) &topomap->selfIdPacket[i];
      decode_selfid(&packet,pselfid_int);
      // find the camera related to this packet:
      for (j=0;j<camera_num;j++)
	if (cameras[j].id==packet.packetZero.phyID)
	  selfids[j]=packet;
    }
}

void SelectCamera(int i)
{
  current_camera=i;
  camera=&cameras[i];
  feature_set=&feature_sets[i];
  misc_info=&misc_infos[i];
  capture=&captures[i];
  format7_info=&format7_infos[i];
  uiinfo=&uiinfos[i];
  selfid=&selfids[i];
}

void MainError(const char *string)
{
  char temp[256];
  sprintf(temp," ERROR: %s",string);
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"main_status"), ctxt.main_ctxt, ctxt.main_id);
  ctxt.main_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"main_status"), ctxt.main_ctxt, temp);

}

void MainStatus(const char *string)
{
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(commander_window,"main_status"), ctxt.main_ctxt, ctxt.main_id);
  ctxt.main_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(commander_window,"main_status"), ctxt.main_ctxt, string);
}

static void MessageBox_clicked (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy( GTK_WIDGET(data));
}

static void MessageBox_destroy (GtkWidget *widget, gpointer data)
{
    gtk_grab_remove (GTK_WIDGET(widget));
}

void MessageBox( gchar *message)
{
  static GtkWidget *label;
  GtkWidget *button;
  GtkWidget *dialog_window;
  
  dialog_window = gtk_dialog_new();
  gtk_signal_connect( GTK_OBJECT(dialog_window), "destroy", GTK_SIGNAL_FUNC(MessageBox_destroy), dialog_window);
  gtk_window_set_title (GTK_WINDOW(dialog_window), "Coriander Message");
  gtk_container_border_width (GTK_CONTAINER(dialog_window), 5);
  label = gtk_label_new (message);
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog_window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  button = gtk_button_new_with_label ("OK");
  gtk_signal_connect (GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(MessageBox_clicked), dialog_window);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog_window)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  gtk_widget_show (dialog_window);
  gtk_grab_add (dialog_window);
}

void
UpdateIdler(void)
{
  // grab latest camera info
  // - ranges
  // - basic capabilities

  // update all windows
}

