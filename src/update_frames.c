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

#include "update_frames.h"

extern GtkWidget *main_window;
extern GtkWidget *preferences_window;
extern CtxtInfo_t ctxt;
extern char* phy_speed_list[4];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern PrefsInfo preferences; 
extern camera_t* camera;
extern camera_t* cameras;
extern cursor_info_t cursor_info;
extern BusInfo_t* businfo;

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
  gtk_widget_set_sensitive(lookup_widget(main_window,"display_redraw_rate"),
			   preferences.display_redraw==DISPLAY_REDRAW_ON);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label155"),
			   preferences.display_redraw==DISPLAY_REDRAW_ON);
}

void
UpdatePrefsReceiveFrame(void)
{
  // thread presence blanking: default some to ON
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_receive_frame"), TRUE);

  // normal:
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_video1394_device"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label84"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_receive_dropframes"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label153"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label154"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);
  gtk_widget_set_sensitive(lookup_widget(main_window,"dma_buffer_size"),
			   preferences.receive_method==RECEIVE_METHOD_VIDEO1394);

  // thread presence balnking:
  if (GetService(camera,SERVICE_ISO)!=NULL)
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_receive_frame"), FALSE);
}

void
UpdatePrefsSaveFrame(void)
{
  // thread presence balnking: default some to ON
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_file_frame"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"use_ram_buffer"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_scratch"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_seq"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_video"), TRUE);

  // normal:
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"prefs_save_noconvert")),
			       preferences.save_scratch==SAVE_SCRATCH_VIDEO);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_convert"),
			   preferences.save_scratch!=SAVE_SCRATCH_VIDEO);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_date_tag"),
			   preferences.save_scratch==SAVE_SCRATCH_SEQUENTIAL);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_num_tag"),
			   preferences.save_scratch==SAVE_SCRATCH_SEQUENTIAL);
  gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), preferences.use_ram_buffer && (preferences.save_scratch==SAVE_SCRATCH_VIDEO));
  gtk_widget_set_sensitive(lookup_widget(main_window,"use_ram_buffer"), preferences.save_scratch==SAVE_SCRATCH_VIDEO);

  // thread presence blanking:
  if (GetService(camera,SERVICE_SAVE)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_file_frame"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(main_window,"use_ram_buffer"), FALSE);
    if (preferences.save_scratch==SAVE_SCRATCH_VIDEO) {
      gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_scratch"), FALSE);
      gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_seq"), FALSE);
    }
    else
      gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_video"), FALSE);
  }
      
    

}


void
UpdatePrefsFtpFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_date_tag"),
			   preferences.ftp_scratch==FTP_SCRATCH_SEQUENTIAL);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_num_tag"),
			   preferences.ftp_scratch==FTP_SCRATCH_SEQUENTIAL);

  // thread presence balnking:
  if (GetService(camera,SERVICE_FTP)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),FALSE);
  }
  else {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),TRUE);
  }
    
}

void
UpdatePrefsV4lFrame(void)
{
  // thread presence balnking:
  if (GetService(camera,SERVICE_V4L)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"v4l_output_device_frame"),FALSE);
  }
  else {
    gtk_widget_set_sensitive(lookup_widget(main_window,"v4l_output_device_frame"),TRUE);
  }
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
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_frame"),TRUE);

  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_external"),
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(main_window,"fps_menu"),
			   !(camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) &&
			   (camera->misc_info.format != FORMAT_SCALABLE_IMAGE_SIZE));
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_mode"),
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on && 
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_polarity"),
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].available &&
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on &&
			   camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].polarity_capable);
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_count"),
			   (camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   ((camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_2)||
			     camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_3));

  gtk_widget_set_sensitive(lookup_widget(main_window, "label16"),
			   (camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].available) &&
			   (camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on) && 
			   ((camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_2)||
			     camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode == TRIGGER_MODE_3));
}

void
UpdatePowerFrame(void)
{
  // nothing to update
}

void
UpdateMemoryFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_channel"),TRUE);

  // save not activated by default (it is not avail. for factory defaults channel):
  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(main_window,"save_mem")),
			   ((camera->misc_info.mem_channel_number>0)&&(camera->misc_info.save_channel>0)));

  // load always present, so we can activate it:
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"load_mem"),TRUE);
}


void
UpdateIsoFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_start"),!camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_restart"),camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_stop"),camera->misc_info.is_iso_on);
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
  char *temp;
  quadlet_t sw_version;
  quadlet_t value[3];

  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  value[0]= camera->camera_info.euid_64 & 0xffffffff;
  value[1]= (camera->camera_info.euid_64 >>32) & 0x000000ff;
  value[2]= (camera->camera_info.euid_64 >>40) & 0xfffff;

  // vendor:
  sprintf(temp," %s",camera->camera_info.vendor);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_vendor_status"), ctxt.vendor_ctxt, ctxt.vendor_id);
  ctxt.vendor_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"camera_vendor_status"), ctxt.vendor_ctxt, temp);

  // camera model:
  sprintf(temp," %s",camera->camera_info.model);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_model_status"), ctxt.model_ctxt, ctxt.model_id);
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(main_window,"camera_model_status"), ctxt.model_ctxt, temp);

  // camera node/bus:
  sw_version=dc1394_get_camera_port(camera->camera_info.handle);
  sprintf(temp," %d  /  %d",camera->camera_info.id, sw_version);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_node_status"), ctxt.node_ctxt, ctxt.node_id);
  ctxt.node_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(main_window,"camera_node_status"), ctxt.node_ctxt, temp);

  // camera handle:
  sprintf(temp," 0x%x",(unsigned int)camera->camera_info.handle);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_handle_status"), ctxt.handle_ctxt, ctxt.handle_id);
  ctxt.handle_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_handle_status"), ctxt.handle_ctxt, temp);

  // camera GUID:
  sprintf(temp," 0x%06x-%02x%08x", value[2], value[1], value[0]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_guid_status"), ctxt.guid_ctxt, ctxt.guid_id);
  ctxt.guid_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_guid_status"), ctxt.guid_ctxt, temp);

  // camera maximal PHY speed:
  sprintf(temp," %s",phy_speed_list[camera->selfid.packetZero.phySpeed]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, ctxt.max_iso_id);
  ctxt.max_iso_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, temp);

  // camera maximal PHY delay:
  sprintf(temp," %s",phy_delay_list[camera->selfid.packetZero.phyDelay]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_delay_status"), ctxt.delay_ctxt, ctxt.delay_id);
  ctxt.delay_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_delay_status"), ctxt.delay_ctxt, temp);

  // IIDC software revision:
  if (dc1394_get_sw_version(camera->camera_info.handle, camera->camera_info.id, &sw_version)!=DC1394_SUCCESS) {
    MainError("Could not get the IIDC software revision");
    sw_version=0x0;
  }
  switch (sw_version) {
  case 0x100: sprintf(temp," 1.04 ");break;
  case 0x101: sprintf(temp," 1.20 ");break;
  case 0x102: sprintf(temp," 1.30 ");break;
  case 0x114: sprintf(temp," Point Grey 114 ");break;
  default: sprintf(temp," ?? 0x%x ",sw_version);
  }
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_dc_status"), ctxt.dc_ctxt, ctxt.dc_id);
  ctxt.dc_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_dc_status"), ctxt.dc_ctxt, temp);

  // power class:
  sprintf(temp," %s",power_class_list[camera->selfid.packetZero.powerClass]);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_pwclass_status"), ctxt.pwclass_ctxt, ctxt.pwclass_id);
  ctxt.pwclass_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_pwclass_status"), ctxt.pwclass_ctxt,temp);

  // camera name: 
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window,"camera_name_text")), camera->name);

  free(temp);

}


void
UpdateTransferStatusFrame(void)
{
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  sprintf(temp," %d",camera->misc_info.iso_channel);
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  if (dc1394_get_iso_channel_and_speed(camera->camera_info.handle, camera->camera_info.id, &camera->misc_info.iso_channel, &camera->misc_info.iso_speed)!=DC1394_SUCCESS)
    MainError("Can't get ISO channel and speed");
  sprintf(temp," %d",camera->misc_info.iso_channel);
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  free(temp);
}

void
UpdateCursorFrame(void)
{
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // position: 
  sprintf(temp," %d,%d",cursor_info.x,cursor_info.y);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_pos"), ctxt.cursor_pos_ctxt, ctxt.cursor_pos_id);
  ctxt.cursor_pos_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_pos"), ctxt.cursor_pos_ctxt, temp);

  // color:
  if (cursor_info.col_r>-255) {
    sprintf(temp," %03d,%03d,%03d",cursor_info.col_r,cursor_info.col_g,cursor_info.col_b);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_rgb"), ctxt.cursor_rgb_ctxt, ctxt.cursor_rgb_id);
    ctxt.cursor_rgb_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_rgb"), ctxt.cursor_rgb_ctxt, temp);
  }
  
  if (cursor_info.col_y>-255) {
    sprintf(temp," %03d,%03d,%03d",cursor_info.col_y,cursor_info.col_u,cursor_info.col_v);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_yuv"), ctxt.cursor_yuv_ctxt, ctxt.cursor_yuv_id);
      ctxt.cursor_yuv_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_yuv"), ctxt.cursor_yuv_ctxt, temp);
  }

  free(temp);
}

void
UpdateOptionFrame(void)
{
  int cond16, cond8, cond422;

  pthread_mutex_lock(&camera->uimutex);
  gtk_widget_set_sensitive(lookup_widget(main_window,"pattern_menu"),
			   camera->bayer!=NO_BAYER_DECODING);
  pthread_mutex_unlock(&camera->uimutex);
  if (camera->misc_info.format!=FORMAT_SCALABLE_IMAGE_SIZE) {
    cond8=((camera->misc_info.mode==MODE_640x480_MONO)||
	   (camera->misc_info.mode==MODE_800x600_MONO)||
	   (camera->misc_info.mode==MODE_1024x768_MONO)||
	   (camera->misc_info.mode==MODE_1280x960_MONO)||
	   (camera->misc_info.mode==MODE_1600x1200_MONO));
    cond16=((camera->misc_info.mode==MODE_640x480_MONO16)||
	    (camera->misc_info.mode==MODE_800x600_MONO16)||
	    (camera->misc_info.mode==MODE_1024x768_MONO16)||
	    (camera->misc_info.mode==MODE_1280x960_MONO16)||
	    (camera->misc_info.mode==MODE_1600x1200_MONO16));
    cond422=((camera->misc_info.mode==MODE_320x240_YUV422)||
	     (camera->misc_info.mode==MODE_640x480_YUV422)||
	     (camera->misc_info.mode==MODE_800x600_YUV422)||
	     (camera->misc_info.mode==MODE_1024x768_YUV422)||
	     (camera->misc_info.mode==MODE_1280x960_YUV422)||
	     (camera->misc_info.mode==MODE_1600x1200_YUV422));
  }
  else {
    cond16=(camera->format7_info.mode[camera->misc_info.mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_MONO16);
    cond8=(camera->format7_info.mode[camera->misc_info.mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_MONO8);
    cond422=(camera->format7_info.mode[camera->misc_info.mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_YUV422);
  }
  gtk_widget_set_sensitive(lookup_widget(main_window,"pattern_menu"),(cond8||cond16||cond422));
  gtk_widget_set_sensitive(lookup_widget(main_window,"bayer_menu"),(cond8||cond16||cond422));
  gtk_widget_set_sensitive(lookup_widget(main_window,"stereo_menu"),cond16||cond422);
  pthread_mutex_lock(&camera->uimutex);
  gtk_widget_set_sensitive(lookup_widget(main_window,"mono16_bpp"),cond16&&
			   (camera->stereo==NO_STEREO_DECODING)&&(camera->bayer==NO_BAYER_DECODING));
  gtk_widget_set_sensitive(lookup_widget(main_window,"label114"),cond16&&
			   (camera->stereo==NO_STEREO_DECODING)&&(camera->bayer==NO_BAYER_DECODING));
  pthread_mutex_unlock(&camera->uimutex);
  
}


void
UpdateServiceFrame(void)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_iso")),
			       GetService(camera,SERVICE_ISO)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_display")),
			       GetService(camera,SERVICE_DISPLAY)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_save")),
			       GetService(camera,SERVICE_SAVE)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_ftp")),
			       GetService(camera,SERVICE_FTP)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_v4l")),
			       GetService(camera,SERVICE_V4L)!=NULL);

}


void
UpdateFormat7InfoFrame(void)
{

  char *temp;
  Format7ModeInfo_t *mode;
  float bpp;
  int bytesize, grandtotal;
  
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  if (camera->format7_info.edit_mode!=-1) {

    mode = &camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN];

    switch (mode->color_coding_id) {
    case COLOR_FORMAT7_MONO8:
      bpp=1;
      break;
    case COLOR_FORMAT7_MONO16:
      bpp=2;
      break;
    case COLOR_FORMAT7_RGB8:
      bpp=3;
      break;
    case COLOR_FORMAT7_RGB16:
      bpp=6;
      break;
    case COLOR_FORMAT7_YUV444:
      bpp=3;
      break;
    case COLOR_FORMAT7_YUV422:
      bpp=2;
      break;
    case COLOR_FORMAT7_YUV411:
      bpp=1.5;
      break;
    default:
      bpp=1;
      MainError("Wrong format_7 color coding ID!");
      break;
    }
    
    bytesize=(int) ((float)mode->size_x*(float)mode->size_y*bpp);
    /*
      // this appears to be meaningless as some cameras take padding into account
      if (bytesize!=mode->total_bytes) {
      fprintf(stderr,"bytesize: %d, total_bytes: %d\n",bytesize, (int)mode->total_bytes);
      MainStatus("The camera has a strange TOTAL_BYTES value.");
      }
    */
    //fprintf(stderr,"total bytes: %d\n",mode->total_bytes);
    if (mode->bpp!=0) {
      // if there is packet padding, take it into account
      if (mode->total_bytes%mode->bpp!=0) {
	grandtotal=(mode->total_bytes/mode->bpp+1)*mode->bpp;
      }
      else {
	grandtotal=mode->total_bytes;
      }
    }
    else {
      grandtotal=0;
      MainError("BPP is zero! This should not happen.");
    }

    sprintf(temp," %d", bytesize);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_imagebytes"), 
			 ctxt.format7_imagebytes_ctxt, ctxt.format7_imagebytes_id);
    ctxt.format7_imagebytes_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_imagebytes"), 
						  ctxt.format7_imagebytes_ctxt,temp);
    
    sprintf(temp," %d", mode->size_x*mode->size_y);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_imagepixels"), 
			 ctxt.format7_imagepixels_ctxt, ctxt.format7_imagepixels_id);
    ctxt.format7_imagepixels_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_imagepixels"), 
						   ctxt.format7_imagepixels_ctxt,temp);
    
    sprintf(temp," %d", grandtotal);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_totalbytes"), 
			 ctxt.format7_totalbytes_ctxt, ctxt.format7_totalbytes_id);
    ctxt.format7_totalbytes_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_totalbytes"), 
						  ctxt.format7_totalbytes_ctxt,temp);
    
    sprintf(temp," %d", grandtotal - bytesize);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_padding"), 
			 ctxt.format7_padding_ctxt, ctxt.format7_padding_id);
    ctxt.format7_padding_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_padding"), 
					       ctxt.format7_padding_ctxt,temp);
    
  }
  free(temp);
}

void
UpdateBandwidthFrame(void)
{
  camera_t* cam;
  unsigned int bandwidth;
  float *ports;
  float ratio;
  char* temp;
  int nports, i, truebps, theobps;
  chain_t* iso_service;
  GtkProgressBar *bar;
  
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // get the number of ports
  nports=businfo->port_num;
  
  ports=(float*)malloc(nports*sizeof(float));

  for (i=0;i<nports;i++)
    ports[i]=0;

  cam=cameras;
  while(cam!=NULL) {
    if (dc1394_get_bandwidth_usage(cam->camera_info.handle, cam->camera_info.id, &bandwidth)!=DC1394_SUCCESS) {
      MainError("Could not get a camera bandwidth usage. Bus usage might be inaccurate.");
    }
    iso_service=GetService(cam,SERVICE_ISO);
    // if we are using format7 and there is a running ISO service, we can get a better estimate:
    if ((cam->misc_info.format==FORMAT_SCALABLE_IMAGE_SIZE)&&(iso_service!=NULL)){
      //fprintf(stderr,"better estimate can be found\n");
      // use the fractions of packets needed:
      theobps=8000*cam->format7_info.mode[cam->format7_info.edit_mode-MODE_FORMAT7_MIN].bpp;
      truebps=iso_service->fps*cam->format7_info.mode[cam->format7_info.edit_mode-MODE_FORMAT7_MIN].total_bytes;
      ratio=(float)truebps/(float)theobps;
      //fprintf(stderr,"truebps: %d, theobps: %d, ratio: %.2f\n",truebps, theobps, ratio);
      // apply only if the ratio is less than 0.95 and greater than 0
      if ((ratio<.95)&&(ratio>0))
	bandwidth=(int)((float)bandwidth*ratio);
    }
    // sum the values of the bandwidths
    ports[dc1394_get_camera_port(cam->camera_info.handle)]+=bandwidth;
    cam=cam->next;
  }

  for (i=0;i<nports;i++) {
    sprintf(temp,"bandwidth_bar%d",i);
    ports[i]=ports[i]/4915;
    if (ports[i]>1.0)
      ports[i]=1;
    //fprintf(stderr,"Cam bandwidth usage: %.1f%%\n",100*ports[i]);
    bar=GTK_PROGRESS_BAR(lookup_widget(main_window,temp));
    gtk_progress_set_percentage(GTK_PROGRESS(bar),ports[i] );
  }
  free(ports);
  free(temp);
 
  UpdateServiceTree();

}

void
UpdateServiceTree(void)
{
  char **temp;
  int i;
  GtkCList *list;
  camera_t* cam;
  chain_t* service;

  list=(GtkCList*)(lookup_widget(main_window,"service_clist"));

  temp=(char**)malloc(3*sizeof(char*));
  for (i=0;i<3;i++) {
    temp[i]=(char*)malloc(STRING_SIZE*sizeof(char));
  }
  
  // freeze the clist
  gtk_clist_freeze(list);
  
  // clear the clist
  gtk_clist_clear(list);

  cam=cameras;
  while(cam!=NULL) {
    sprintf(temp[0],"%s",cam->name);
    sprintf(temp[1]," ");
    sprintf(temp[2]," ");
    gtk_clist_append (list, temp);
    for (i=SERVICE_ISO;i<=SERVICE_FTP;i++) {
      service=GetService(cam,i);
      if (service!=NULL) {
	switch(i) {
	  case 0: sprintf(temp[0],"     Receive");break;
	  case 1: sprintf(temp[0],"     Display");break;
	  case 2: sprintf(temp[0],"     Save   ");break;
	  case 3: sprintf(temp[0],"     V4L    ");break;
	  case 4: sprintf(temp[0],"     FTP    ");break;
	  default: sprintf(temp[0],"!! Unknown service ID !!");break;
	}
	sprintf(temp[1],"%.5f", service->fps);
	sprintf(temp[2],"%llu", service->processed_frames);
	gtk_clist_append (list, temp);
      }
    }
    cam=cam->next;
  }

  gtk_clist_set_column_width (list, 0, 200);

  // unfreeze the clist
  gtk_clist_thaw(list);

  for (i=0;i<3;i++) {
    free(temp[i]);
  }
  free(temp);
}
