/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#include "tools.h"
  
#define YUV2RGB(y, u, v, r, g, b)\
  r = y + ((v*1436) >>10);\
  g = y - ((u*352 + v*731) >> 10);\
  b = y + ((u*1814) >> 10);\
  r = r < 0 ? 0 : r;\
  g = g < 0 ? 0 : g;\
  b = b < 0 ? 0 : b;\
  r = r > 255 ? 255 : r;\
  g = g > 255 ? 255 : g;\
  b = b > 255 ? 255 : b

extern GtkWidget *main_window;
extern GtkWidget *absolute_settings_window;
extern char* feature_abs_entry_list[NUM_FEATURES];
extern char* feature_abs_switch_list[NUM_FEATURES];
extern char* feature_abs_label_list[NUM_FEATURES];
extern char* trigger_mode_list[4];
extern char* channel_num_list[16];
extern char* phy_speed_list[4];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern camera_t* camera;
extern camera_t* cameras;
extern int camera_num;
extern CtxtInfo ctxt;
extern raw1394handle_t* handles;
extern unsigned int main_timeout_ticker;
extern int WM_cancel_display;
extern cursor_info_t cursor_info;

void
GetFormat7Capabilities(raw1394handle_t handle, nodeid_t node, Format7Info *info)
{
  int i, f;
  quadlet_t value;
  
  if (dc1394_query_supported_modes(handle, node, FORMAT_SCALABLE_IMAGE_SIZE, &value)!=DC1394_SUCCESS)
    MainError("Could not query Format7 supported modes");
  else {
    for (i=0,f=MODE_FORMAT7_MIN;f<MODE_FORMAT7_MAX;f++,i++) {
      info->mode[i].present= (value & (0x1<<(31-i)) );
      if (info->mode[i].present) { // check for mode presence before query
	if (dc1394_query_format7_max_image_size(handle,node,f,&info->mode[i].max_size_x,&info->mode[i].max_size_y)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 max image size");
	if (dc1394_query_format7_unit_size(handle,node,f,&info->mode[i].step_x,&info->mode[i].step_y)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 unit size");
	// quick hack to keep size/position even. If pos/size is ODD, strange color/distorsions occur on some cams
	// (e.g. Basler cams). This will have to really fixed later.
	// REM: this is fixed by using the unit_position:
	//fprintf(stderr,"Using pos units = %d %d\n",info->mode[i].step_pos_x,info->mode[i].step_pos_y);
	if (dc1394_query_format7_unit_position(handle,node,f,&info->mode[i].step_pos_x,&info->mode[i].step_pos_y)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 unit position");
	info->mode[i].use_unit_pos=((info->mode[i].step_pos_x>0)&&(info->mode[i].step_pos_x<info->mode[i].max_size_x)&&
				    (info->mode[i].step_pos_y>0)&&(info->mode[i].step_pos_y<info->mode[i].max_size_y));
	
	if (dc1394_query_format7_image_position(handle,node,f,&info->mode[i].pos_x,&info->mode[i].pos_y)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 image position");
	if (dc1394_query_format7_image_size(handle,node,f,&info->mode[i].size_x,&info->mode[i].size_y)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 image size");
	if (dc1394_query_format7_pixel_number(handle,node,f,&info->mode[i].pixnum)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 pixel number");
	if (dc1394_query_format7_byte_per_packet(handle,node,f,&info->mode[i].bpp)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 bytes per packet");
	if (dc1394_query_format7_packet_para(handle,node,f,&info->mode[i].min_bpp,&info->mode[i].max_bpp)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 packet parameters");
	if (dc1394_query_format7_total_bytes(handle,node,f,&info->mode[i].total_bytes)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 total bytes per frame");
	if (dc1394_query_format7_color_coding_id(handle,node,f,&info->mode[i].color_coding_id)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 color coding ID");
	if (dc1394_query_format7_color_coding(handle,node,f,&info->mode[i].color_coding)!=DC1394_SUCCESS)
	  MainError("Got a problem querying format7 color coding");
	
	//fprintf(stderr,"color coding for mode %d: 0x%x, current: %d\n", i,
	//	      info->mode[i].color_coding, info->mode[i].color_coding_id);
	
      }
    }
  }
  //info->edit_mode = MODE_FORMAT7_MIN;
}

void
ChangeModeAndFormat         (GtkMenuItem     *menuitem,
			     gpointer         user_data)
{
  int state;
  int format;

  int mode;
  
  mode=(int)user_data;

  //fprintf(stderr,"Mode: %d\n",mode);

  if ((mode>=MODE_FORMAT0_MIN)&&(mode<=MODE_FORMAT0_MAX))
    format=FORMAT_VGA_NONCOMPRESSED;
  else
    if ((mode>=MODE_FORMAT1_MIN)&&(mode<=MODE_FORMAT1_MAX))
      format=FORMAT_SVGA_NONCOMPRESSED_1;
    else
      if ((mode>=MODE_FORMAT2_MIN)&&(mode<=MODE_FORMAT2_MAX))
	format=FORMAT_SVGA_NONCOMPRESSED_2;
      else
	if ((mode>=MODE_FORMAT6_MIN)&&(mode<=MODE_FORMAT6_MAX))
	  format=FORMAT_STILL_IMAGE;
	else
	  format=FORMAT_SCALABLE_IMAGE_SIZE;

  IsoFlowCheck(&state);

  if (dc1394_set_video_format(camera->camera_info.handle,camera->camera_info.id,format)!=DC1394_SUCCESS)
    MainError("Could not set video format");
  else
    camera->misc_info.format=format;

  if (dc1394_set_video_mode(camera->camera_info.handle,camera->camera_info.id,mode)!=DC1394_SUCCESS)
    MainError("Could not set video mode");
  else
    camera->misc_info.mode=mode;
 
  IsoFlowResume(&state);

  // REPROBE EVERYTHING
  if (dc1394_get_camera_info(camera->camera_info.handle,camera->camera_info.id, &camera->camera_info)!=DC1394_SUCCESS)
    MainError("Could not get camera basic information!");
  if (dc1394_get_camera_misc_info(camera->camera_info.handle,camera->camera_info.id, &camera->misc_info)!=DC1394_SUCCESS)
    MainError("Could not get camera misc information!");
  if (dc1394_get_camera_feature_set(camera->camera_info.handle,camera->camera_info.id, &camera->feature_set)!=DC1394_SUCCESS)
    MainError("Could not get camera feature information!");

  if (format==FORMAT_SCALABLE_IMAGE_SIZE) {
    GetFormat7Capabilities(camera->camera_info.handle,camera->camera_info.id, &camera->format7_info);
    //format7_info->edit_mode=mode;
  }

  BuildAllWindows();
  UpdateAllWindows();
}


void IsoFlowCheck(int *state)
{ 
  if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id, &camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
    MainError("Could not get ISO status");
  else {
    if (camera->misc_info.is_iso_on>0) {
      if (dc1394_stop_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
	// ... (if not done, restarting is no more possible)
	MainError("Could not stop ISO transmission");
    }
  }
  // memorize state:
  *state=(GetService(SERVICE_ISO)!=NULL);
  if (*state!=0) {
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window,"service_iso"),FALSE);
  }
}

void IsoFlowResume(int *state)
{
  int was_on;

  was_on=camera->misc_info.is_iso_on;
  if (was_on>0) { // restart if it was 'on' before the changes
    usleep(50000); // necessary to avoid desynchronized ISO flow.
    if (dc1394_start_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
      MainError("Could not start ISO transmission");
  }

  if (*state!=0) {
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window,"service_iso"),TRUE);
  }
  
  if (was_on>0) {
    if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id,&camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
      MainError("Could not get ISO status");
    else {
      if (!camera->misc_info.is_iso_on) {
	MainError("ISO not properly restarted. Trying again after 1 second");
	usleep(1000000);
	if (dc1394_start_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
	  // ... (if not done, restarting is no more possible)
	  MainError("Could not start ISO transmission");
	else {
	  if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id,&camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
	    MainError("Could not get ISO status");
	  else
	    if (!camera->misc_info.is_iso_on)
	      MainError("Can't start ISO, giving up...");
	}
      }
    }
    UpdateIsoFrame();
  }
}

void GetContextStatus()
{
  ctxt.model_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_model_status"),"");
  ctxt.vendor_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_vendor_status"),"");
  ctxt.handle_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_handle_status"),"");
  ctxt.node_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_node_status"),"");
  ctxt.guid_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_guid_status"),"");
  ctxt.max_iso_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_maxiso_status"),"");
  ctxt.delay_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_delay_status"),"");
  ctxt.dc_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_dc_status"),"");
  ctxt.pwclass_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"camera_pwclass_status"),"");

  ctxt.iso_channel_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"),"");

  // init message ids.
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_model_status"), ctxt.model_ctxt, "");
  ctxt.vendor_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_vendor_status"), ctxt.vendor_ctxt, "");
  ctxt.handle_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_handle_status"), ctxt.handle_ctxt, "");
  ctxt.node_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_node_status"), ctxt.node_ctxt, "");
  ctxt.guid_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_guid_status"), ctxt.guid_ctxt, "");
  ctxt.max_iso_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, "");
  ctxt.delay_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_delay_status"), ctxt.delay_ctxt, "");
  ctxt.dc_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_dc_status"), ctxt.dc_ctxt, "");
  ctxt.pwclass_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"camera_pwclass_status"), ctxt.pwclass_ctxt, "");

  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, "");

  // note: these empty messages will be replaced after the execution of update_frame for status window

  ctxt.cursor_pos_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"cursor_pos"),"");
  ctxt.cursor_rgb_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"cursor_rgb"),"");
  ctxt.cursor_yuv_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"cursor_yuv"),"");

  ctxt.cursor_pos_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"cursor_pos"), ctxt.cursor_pos_ctxt, "");
  ctxt.cursor_rgb_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"cursor_rgb"), ctxt.cursor_rgb_ctxt, "");
  ctxt.cursor_yuv_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"cursor_yuv"), ctxt.cursor_yuv_ctxt, "");


  // FPS:
  ctxt.fps_receive_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"fps_receive"),"");
  ctxt.fps_display_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"fps_display"),"");
  ctxt.fps_save_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"fps_save"),"");
  ctxt.fps_ftp_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"fps_ftp"),"");
  ctxt.fps_v4l_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"fps_v4l"),"");

  ctxt.fps_receive_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"fps_receive"), ctxt.fps_receive_ctxt, "");
  ctxt.fps_display_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"fps_display"), ctxt.fps_display_ctxt, "");
  ctxt.fps_save_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"fps_save"), ctxt.fps_save_ctxt, "");
  ctxt.fps_ftp_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"fps_ftp"), ctxt.fps_ftp_ctxt, "");
  ctxt.fps_v4l_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"fps_v4l"), ctxt.fps_v4l_ctxt, "");
  
}

void GrabSelfIds(raw1394handle_t* handles, int portmax)
{
 
  RAW1394topologyMap *topomap;
  SelfIdPacket_t packet;
  unsigned int* pselfid_int;
  int i, port;
  camera_t* camera_ptr;

  for (port=0;port<portmax;port++) {
    if (handles[port]!=0) {
      // get and decode SelfIds.
      topomap=raw1394GetTopologyMap(handles[port]);
      
      for (i=0;i<topomap->selfIdCount;i++) {
	pselfid_int = (unsigned int *) &topomap->selfIdPacket[i];
	decode_selfid(&packet,pselfid_int);
	// find the camera related to this packet:
	
	camera_ptr=cameras;
	while (camera_ptr!=NULL) {
	  if (camera_ptr->camera_info.id==packet.packetZero.phyID) {
	    camera_ptr->selfid=packet;
	  }
	  camera_ptr=camera_ptr->next;
	}
      }
    }
  }
 
}


void
SetChannels(void)
{
  unsigned int channel, speed;
  camera_t* camera_ptr;

  camera_ptr=cameras;
  while(camera_ptr!=NULL) {
    if (dc1394_get_iso_channel_and_speed(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, &channel, &speed)!=DC1394_SUCCESS)
      MainError("Can't get iso channel and speed");
    if (dc1394_set_iso_channel_and_speed(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, camera_ptr->camera_info.id, speed)!=DC1394_SUCCESS)
      MainError("Can't set iso channel and speed");
    camera_ptr=camera_ptr->next;
  }
}

void MainError(const char *string)
{
  char temp[STRING_SIZE];
  sprintf(temp,"ERROR: %s\n",string);
  if (main_window !=NULL) {
    gtk_text_insert((GtkText*)lookup_widget(main_window,"main_status"), NULL,NULL,NULL,temp,-1);
  }
}

void MainStatus(const char *string)
{
  char temp[STRING_SIZE];
  sprintf(temp,"%s\n",string);
  if (main_window !=NULL) {
    gtk_text_insert((GtkText*)lookup_widget(main_window,"main_status"), NULL,NULL,NULL,temp,-1);
  }
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
SetScaleSensitivity(GtkWidget* widget, int feature, dc1394bool_t sense)
{ 
  char stemp[256];

  switch (feature) {
  case FEATURE_WHITE_BALANCE:
    sprintf(stemp,"feature_%d_bu_scale",feature);
    gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), stemp)),sense);
    sprintf(stemp,"feature_%d_rv_scale",feature);
    gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), stemp)),sense);
    break;
  case FEATURE_TEMPERATURE:
    sprintf(stemp,"feature_%d_target_scale",feature);
    gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), stemp)),sense);
    break;
  default:
    sprintf(stemp,"feature_%d_scale",feature);
    gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), stemp)),sense);
    break;
  }
}

/*
void*
AutoWhiteBalance(void* arg)
{
  whitebal_data_t *info;
  int currentB=0, currentR=0;
  int span,center;
  int prevB, prevR;
  int pixB, pixR;
  int prevpixB, prevpixR;
  float kB, kR, oB, oR;
  unsigned char *prev_ptr;
  int target;
  int progressB=1, progressR=1;
  int px, py;
  chain_t *service;

  info=(whitebal_data_t*)arg;

  service=info->service;
  px=info->x;
  py=info->y;

  fprintf(stderr,"Entering Auto White Balance...\n");

  prevB=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value;
  prevR=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value;

  fprintf(stderr,"grabbing RGB...\n");

  GetRGBPix(px, py, service, &prevpixR, &target, &prevpixB);

  fprintf(stderr,"RGB initial value: %d %d %d\n",prevpixR, target, prevpixB);

  span=(feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max-
	feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min);
  center=((feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max-
	   feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min)/2+
	  feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min);

  fprintf(stderr,"span: %d, center: %d\n",span,center);
  fprintf(stderr,"current whitebal: %d %d\n",prevB, prevR);

  // move 10%*range from current value towards center.
  if (prevB>center)
    currentB=prevB-span/10;
  else
    currentB=prevB+span/10;

  if (prevR>center)
    currentR=prevR-span/10;
  else
    currentR=prevR+span/10;
  fprintf(stderr,"first whitebal shift: %d %d\n",currentB, currentR);

  while (progressB||progressR)
    {
      fprintf(stderr,"Set whitebal to %d %d\n",currentB, currentR);
      // get pixel 'color' value (R and B for RGB, U and V for YUV)
      if (dc1394_set_white_balance(camera->handle,camera->id,currentB,currentR)!=DC1394_SUCCESS)
	{
	  MainError("Can't set whitebal parameters in auto white balance proc");
	  return(0);
	}

      // wait for buffer pointer to change
      prev_ptr=service->current_buffer;
      //fprintf(stderr,"prev buffer: 0x%x\n",prev_ptr);
      while (prev_ptr==service->current_buffer)
	usleep(10000);// .01 sec
      
      // re-grab pixel value (and average with 3x3 mask??)
      GetRGBPix(px,py,service, &pixR, &target, &pixB);
      fprintf(stderr,"New RGB values: %d %d %d\n",pixR,target,pixB);
     
      // estimate linear law:
      kB=(pixB-prevpixB)/(currentB-prevB);
      kR=(pixR-prevpixR)/(currentR-prevR);
      oB=pixB-kB*currentB;
      oR=pixR-kR*currentR;
      
      // memorize data as old
      prevpixB=pixB;
      prevpixR=pixR;
      prevB=currentB;
      prevR=currentR;

      // make an prediction:
      currentB=(target-oB)/kB;
      currentR=(target-oR)/kR;

      // clamp values in range limit:
      if (currentB>feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max)
	currentB=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max;
      if (currentB<feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min)
	currentB=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min;
      if (currentR>feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max)
	currentR=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].max;
      if (currentR<feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min)
	currentR=feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].min;

      // update condition
      progressB=(abs(pixB-target)<abs(prevpixB-target));
      progressR=(abs(pixR-target)<abs(prevpixR-target));
    }

  fprintf(stderr,"Leaving Auto White Balance\n");

  return(0);
}


void
GetRGBPix(int px, int py, chain_t *service, int* R, int* G, int* B)
{
  int u, y, v;
  displaythread_info_t *info=NULL;
  info=(displaythread_info_t*)service->data;

  //pthread_mutex_lock(&service->mutex_struct);
  //pthread_mutex_lock(&service->mutex_data);

  // we work in display thread, therefor the format is always YUYV
  y=0;//info->SDL_overlay->pixels[0][(py*service->width+px)*2];
  u=0;//info->SDL_overlay->pixels[0][(((py*service->width+px)>>1)<<2)+1]-127;
  v=0;//info->SDL_overlay->pixels[0][(((py*service->width+px)>>1)<<2)+3]-127;

  //pthread_mutex_unlock(&service->mutex_data);
  //pthread_mutex_unlock(&service->mutex_struct);

  YUV2RGB(y,u,v,*R,*G,*B);
  fprintf(stderr,"YUV: %d %d %d RGB: %d %d %d\n",y,u,v,*R,*G,*B);
}
*/

void
SetAbsoluteControl(int feature, int power)
{
  char string[256];

  if (dc1394_absolute_setting_on_off(camera->camera_info.handle, camera->camera_info.id, feature, power)!=DC1394_SUCCESS)
    MainError("Could not activate absolute setting\n");
  else {
    camera->feature_set.feature[feature-FEATURE_MIN].abs_control=power;
    sprintf(string,"feature_%d_frame",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, string), !power);
    gtk_widget_set_sensitive(lookup_widget(absolute_settings_window,feature_abs_entry_list[feature-FEATURE_MIN]),power);
    gtk_widget_set_sensitive(lookup_widget(absolute_settings_window,feature_abs_label_list[feature-FEATURE_MIN]),power);
    if (power>0) {
      // update absolute value 
      dc1394_query_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, &camera->feature_set.feature[feature-FEATURE_MIN].abs_value);
      sprintf(string,"%f",camera->feature_set.feature[feature-FEATURE_MIN].abs_value);
      gtk_entry_set_text(GTK_ENTRY(lookup_widget(absolute_settings_window, feature_abs_entry_list[feature-FEATURE_MIN])),
			 string);
    }
    else {
      // update range
      UpdateRange(feature);
    }
  }
  
}


dc1394bool_t
BuildAbsControl(int feature)
{
  dc1394bool_t capable, working;
  char string[256];

  capable=camera->feature_set.feature[feature-FEATURE_MIN].absolute_capable;
  gtk_widget_set_sensitive(lookup_widget(absolute_settings_window,feature_abs_switch_list[feature-FEATURE_MIN]),capable);
  if (capable) {
    sprintf(string,"%f",camera->feature_set.feature[feature-FEATURE_MIN].abs_value);
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(absolute_settings_window, feature_abs_entry_list[feature-FEATURE_MIN])),
		       string);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(absolute_settings_window, feature_abs_switch_list[feature-FEATURE_MIN])),
				 camera->feature_set.feature[feature-FEATURE_MIN].abs_control);
  }
  working=(capable&&camera->feature_set.feature[feature-FEATURE_MIN].abs_control);
  gtk_widget_set_sensitive(lookup_widget(absolute_settings_window,feature_abs_entry_list[feature-FEATURE_MIN]),working);
  gtk_widget_set_sensitive(lookup_widget(absolute_settings_window,feature_abs_label_list[feature-FEATURE_MIN]),working);

  return capable;
}

void
SetAbsValue(int feature)
{
  char string[256];
  char *stringp;
  float value;
 
  stringp=gtk_entry_get_text(GTK_ENTRY(lookup_widget(absolute_settings_window,feature_abs_entry_list[feature-FEATURE_MIN])));
  value=atof(stringp);
  if (dc1394_set_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, value)!=DC1394_SUCCESS) {
    MainError("Can't set absolute value!");
  }
  else {
    if (dc1394_query_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, &value)!=DC1394_SUCCESS) {
      MainError("Can't get absolute value!");
    }
    else
      {
	sprintf(string,"%.8f",value);
	gtk_entry_set_text(GTK_ENTRY(lookup_widget(absolute_settings_window,feature_abs_entry_list[feature-FEATURE_MIN])),string);
      }
  }
}

void
GetAbsValue(int feature)
{
  char string[256];
  float value;
 
  
  if (dc1394_query_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, &value)!=DC1394_SUCCESS) {
    MainError("Can't get absolute value!");
  }
  else {
    sprintf(string,"%.8f",value);
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(absolute_settings_window,feature_abs_entry_list[feature-FEATURE_MIN])),string);
  }
}


void
StopFPSDisplay(void)
{
  chain_t *service;
  isothread_info_t* infoiso;
  //displaythread_info_t* infodisplay;
  savethread_info_t* infosave;
  ftpthread_info_t* infoftp;
  v4lthread_info_t* infov4l;

  service=GetService(SERVICE_ISO);
  if (service!=NULL) {
    infoiso=(isothread_info_t*)service->data;
    //fprintf(stderr,"Stopping iso fps, id %d...",infoiso->timeout_func_id);
    gtk_timeout_remove(infoiso->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_receive"),
			 ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_receive"),
					   ctxt.fps_receive_ctxt, "");
    //fprintf(stderr,"done\n");
  
  } 
  // we don't need to stop display FPS: the thread is completely disabled if necessary.
  
  service=GetService(SERVICE_SAVE);
  if (service!=NULL) {
    infosave=(savethread_info_t*)service->data;
    //fprintf(stderr,"Stopping save fps, id %d...",infosave->timeout_func_id);
    gtk_timeout_remove(infosave->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_save"),
			 ctxt.fps_save_ctxt, ctxt.fps_save_id);
    ctxt.fps_save_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_save"),
					   ctxt.fps_save_ctxt, "");
    //fprintf(stderr,"done\n");
  }
  service=GetService(SERVICE_FTP);
  if (service!=NULL) {
    infoftp=(ftpthread_info_t*)service->data;
    //fprintf(stderr,"Stopping ftp fps, id %d...",infoftp->timeout_func_id);
    gtk_timeout_remove(infoftp->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_ftp"),
			 ctxt.fps_ftp_ctxt, ctxt.fps_ftp_id);
    ctxt.fps_ftp_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_ftp"),
					   ctxt.fps_ftp_ctxt, "");
    //fprintf(stderr,"done\n");
  }
  service=GetService(SERVICE_V4L);
  if (service!=NULL) {
    infov4l=(v4lthread_info_t*)service->data;
    //fprintf(stderr,"Stopping v4l fps, id %d...",infov4l->timeout_func_id);
    gtk_timeout_remove(infov4l->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_v4l"),
			 ctxt.fps_v4l_ctxt, ctxt.fps_v4l_id);
    ctxt.fps_v4l_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_v4l"),
					   ctxt.fps_v4l_ctxt, "");
    //fprintf(stderr,"done\n");
  }
}

void
ResumeFPSDisplay(void)
{
  chain_t *service;
  isothread_info_t* infoiso;
  //displaythread_info_t* infodisplay;
  savethread_info_t* infosave;
  ftpthread_info_t* infoftp;
  v4lthread_info_t* infov4l;

  service=GetService(SERVICE_ISO);
  if (service!=NULL) {
    infoiso=(isothread_info_t*)service->data;
    //fprintf(stderr,"Starting iso fps...");
    infoiso->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)IsoShowFPS, (gpointer*) service);
    //fprintf(stderr,"done: id= %d\n",infoiso->timeout_func_id);
  } 
  // we don't restart display FPS because if necessary the thread will be restarted anyway.
  
  service=GetService(SERVICE_SAVE);
  if (service!=NULL) {
    infosave=(savethread_info_t*)service->data;
    //fprintf(stderr,"Starting save fps...");
    infosave->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)SaveShowFPS, (gpointer*) service);
    //fprintf(stderr,"done: id= %d\n",infosave->timeout_func_id);
  }
  service=GetService(SERVICE_FTP);
  if (service!=NULL) {
    infoftp=(ftpthread_info_t*)service->data;
    //fprintf(stderr,"Starting ftp fps...");
    infoftp->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)FtpShowFPS, (gpointer*) service);
    //fprintf(stderr,"done: id= %d\n",infoftp->timeout_func_id);
  }
  service=GetService(SERVICE_V4L);
  if (service!=NULL) {
    infov4l=(v4lthread_info_t*)service->data;
    //fprintf(stderr,"Starting v4l fps...");
    infov4l->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)V4lShowFPS, (gpointer*) service);
    //fprintf(stderr,"done: id= %d\n",infov4l->timeout_func_id);
  }
}

/*
  The timeout/bus_reset handling technique is 'strongly inspired' by the code found in
  GScanbus by Andreas Micklei.
*/

int
bus_reset_handler(raw1394handle_t handle, unsigned int generation) {

  fprintf(stderr,"Bus reset detected by gtk timeout. Generation: %d\n",generation);
  raw1394_update_generation(handle, generation);
  // Now we have to deal with this bus reset...
  fprintf(stderr,"Coriander does not support hotplugging yet and has therefore entered\n");
  fprintf(stderr,"a very unstable state. This program will self-terminate in 5 seconds.\n");
  usleep(5000000);
  fprintf(stderr,"Bye...\n");
  exit(0);

  return(0);
}

int
main_timeout_handler(gpointer* port_num) {

  int i;
  int ports=(int)port_num;
  quadlet_t quadlet;

  // the main timeout performs tasks every ms. In order to have less repeated tasks
  // the main_timeout_ticker can be consulted.
  main_timeout_ticker=(main_timeout_ticker+10)%1000;

  // -------------------------------------------------------
  // cancel display thread if asked by the SDL/WM
  // We must do this here because it is not allowed to call a GTK function from a thread. At least if we do
  // so the program progressively breaks with strange GUI behavior/look.
  if (!(main_timeout_ticker%100)) { // every 100ms
    if (WM_cancel_display>0) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_display")), FALSE);
      WM_cancel_display=0;
    }
    //fprintf(stderr,"check display cancel\n");
  }

  // -------------------------------------------------------
  // performs a dummy read on all handles
  if (!(main_timeout_ticker%1000)) { // every second
    for (i=0;i<ports;i++) {
      cooked1394_read(handles[i], 0xffc0 | raw1394_get_local_id(handles[i]),
		      CSR_REGISTER_BASE + CSR_CYCLE_TIME, 4,
		      (quadlet_t *) &quadlet);
    }
    //fprintf(stderr,"dummy read\n");
  }
  // -------------------------------------------------------
  if (!(main_timeout_ticker%100)) { // every 100ms
    if (cursor_info.update_req>0) {
      UpdateCursorFrame();
      cursor_info.update_req=0;
    }
  }
  return(1);
}
