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
  
#ifdef HAVE_GDK_PIXBUF

static char * coriander_icon_xpm[] = {
"48 48 97 2",
"  	c None",
". 	c #FF3333",
"+ 	c #47FF47",
"@ 	c #FF5151",
"# 	c #FF0000",
"$ 	c #FFE974",
"% 	c #FFE351",
"& 	c #FFDD26",
"* 	c #FFE248",
"= 	c #FFE76B",
"- 	c #00FF00",
"; 	c #FF1414",
"> 	c #FFDE2E",
", 	c #FFE662",
"' 	c #3DFF3D",
") 	c #0AFF0A",
"! 	c #FF0A0A",
"~ 	c #FF6E2C",
"{ 	c #ADEB33",
"] 	c #FF6110",
"^ 	c #A3E918",
"/ 	c #7AFF7A",
"( 	c #FF0801",
"_ 	c #FFB91F",
": 	c #FFDF37",
"< 	c #FFEA7C",
"[ 	c #FFE140",
"} 	c #F4DE24",
"| 	c #1EFA04",
"1 	c #FF4F0D",
"2 	c #FFE55A",
"3 	c #8EEB15",
"4 	c #14FF14",
"5 	c #28FF28",
"6 	c #FF4747",
"7 	c #1EFF1E",
"8 	c #FF3D3D",
"9 	c #70FF70",
"0 	c #FF7F2F",
"a 	c #0AFD01",
"b 	c #70F010",
"c 	c #FF2828",
"d 	c #FFED8E",
"e 	c #5BFF5B",
"f 	c #5BF20D",
"g 	c #D6E21F",
"h 	c #FFCB22",
"i 	c #FF7B15",
"j 	c #FFD424",
"k 	c #3DF609",
"l 	c #FF5B5B",
"m 	c #FF1A04",
"n 	c #FFA71C",
"o 	c #E0E121",
"p 	c #33F807",
"q 	c #FFEB85",
"r 	c #EACB22",
"s 	c #473D0A",
"t 	c #332C07",
"u 	c #3D3509",
"v 	c #7A6A12",
"w 	c #D6B91F",
"x 	c #A38D18",
"y 	c #0A0801",
"z 	c #000000",
"A 	c #847213",
"B 	c #C1E51C",
"C 	c #AD9619",
"D 	c #141103",
"E 	c #C1A71C",
"F 	c #66580F",
"G 	c #E0C221",
"H 	c #51460C",
"I 	c #F4D424",
"J 	c #F4DD6B",
"K 	c #EAD262",
"L 	c #1E1AE4",
"M 	c #332CD3",
"N 	c #2823DC",
"O 	c #0000FF",
"P 	c #5151FF",
"Q 	c #3333FF",
"R 	c #0A0AFF",
"S 	c #6666FF",
"T 	c #AD9B8E",
"U 	c #7A6EB0",
"V 	c #6658A8",
"W 	c #1411ED",
"X 	c #E0C240",
"Y 	c #CCB051",
"Z 	c #99847C",
"` 	c #7A6A96",
" .	c #473DC2",
"..	c #2826ED",
"+.	c #5B5BFF",
"@.	c #4747FF",
"#.	c #1414FF",
"                                                                                                ",
"                                                                                                ",
"                                                                                                ",
"                        .                                             +                         ",
"                    @ # #             $ % & & & & & & * =             - -                       ",
"                  ; # # # ;     = > & & & & & & & & & & & & & ,     ' - - )                     ",
"                ! # # # # # ~ & & & & & & & & & & & & & & & & & & { - - - - -                   ",
"              # # # # # # # # ] & & & & & & & & & & & & & & & & ^ - - - - - - - - /             ",
"            ! # # # # # # # # ( _ & & : =             < [ & & } | - - - - - - - - -             ",
"          ; # # # # # # # # # # 1 2                         $ 3 - - - - - - - - - - -           ",
"        @ # # # # # # # # # # # #                             4 - - - - - - - - - - - 5         ",
"        # # # # # # # # # # # # # @                         / - - - - - - - - - - - - - /       ",
"      6 # # # # # # # # # # # # # #                         ) - - - - - - - - - - - - - 7       ",
"          ; # # # # # # # # # # # # 8                     9 - - - - - - - - - - - - ) /         ",
"            0 # # # # # # # # # # # #                     ) - - - - - - - - - - a b             ",
"            & ] ( # # # # # # # # # # c     : & > % d   e - - - - - - - - - - f g &             ",
"          = & & h 1 # # # # # # # # ( i j & & & & & & & ^ | - - - - - - - k g & & & [           ",
"          > & & & 2   l # # # # # m n & & & & & & & & & & o p - - - - - ' q & & & & &           ",
"          & & & &         8 # # ( n & & & r s t u v w & & & o | - - 5       & & & & & d         ",
"        $ & & & :             c i & & & x y z z z z z A & & & B 4           2 & & & & *         ",
"        % & & & =               j & & x z z z t y z z z v & & &               & & & : 2         ",
"        & & & &                 & & r y z y & & & C D z z E & & =                               ",
"        & & & &               : & & s z z & & & & & & & & & & & &                               ",
"        & & & &               & & & t z t & & & & & & & & & & & &                               ",
"        & & & &               > & & u z y & & & & & & & & & & & &                               ",
"        & & & &               % & & v z z C & & & & & & & & & & &                               ",
"        & & & &               d & & w z z D & & & w F F F G & & ,                               ",
"        * & & & <               & & & A z z t F u y z z H & & &               & & : , q         ",
"        = & & & [               $ & & & v z z z z z z H I & & *             , & & & & [         ",
"          & & & &                 [ & & & E D z y s C & & & >               > & & & & <         ",
"          & & & & $                 [ & & & & & & & & & & >                 & & & & &           ",
"          , & & & &                   2 & & & & & & & & *                 : & & & & :           ",
"            & & & & ,                     J & & & & K                     & & & & & q           ",
"            * & & & &                       L M N O P                     % & & & >             ",
"              & & & & &                   Q O O O O R                       d & & d             ",
"              & & & & & >                 O O O O O O                           =               ",
"              [ & & & & & >               O O O O O O S                                         ",
"                  & & & & & & & > q     R O O O O O O O       : & %                             ",
"                    & & & & & & & & & T O O O O O O O O U > & & & & d                           ",
"                      : & & & & & & & V O O O O O O O O M & & & & & &                           ",
"                        , & & & & & & W O O O O O O O O O X & & & & & ,                         ",
"                            : & & & Y O O O O O O O O O O Z & & & > d                           ",
"                                [ & ` O O O O O O O O O O  .& : q                               ",
"                                    ..O O O O O O O O O O O                                     ",
"                                    O O O O O O O O O O O O                                     ",
"                                    O O O O O O O O O O O O +.                                  ",
"                                  @.O O O O O O O O O O O O #.                                  ",
"                                      S Q R O O O O O Q +.                                      "};

#endif

extern char* trigger_mode_list[4];
extern char* channel_num_list[16];
extern char* phy_speed_list[7];
extern char* phy_delay_list[4];
extern char* power_class_list[8];
extern char* fps_label_list[NUM_FRAMERATES];

void
GetFormat7Capabilities(camera_t* cam)
{
  int i, f;
  quadlet_t value;
  
  if (dc1394_query_supported_formats(cam->camera_info.handle, cam->camera_info.id, &value)!=DC1394_SUCCESS)
    MainError("Could not query supported formats");
  else {
    if (value & (0x1<<24)) { // is format7 supported?
      if (dc1394_query_supported_modes(cam->camera_info.handle, cam->camera_info.id, FORMAT_SCALABLE_IMAGE_SIZE, &value)!=DC1394_SUCCESS) {
	MainError("Could not query Format7 supported modes");
      }
      else {
	for (f=MODE_FORMAT7_MIN;f<=MODE_FORMAT7_MAX;f++) {
	  cam->format7_info.mode[f-MODE_FORMAT7_MIN].present= (value & (0x1<<(31-(f-MODE_FORMAT7_MIN))) );
	  GetFormat7ModeInfo(cam, f);
	}
      }
    }
    else { // format7 is not supported!!
      for (i=0,f=MODE_FORMAT7_MIN;f<=MODE_FORMAT7_MAX;f++,i++) {
	cam->format7_info.mode[i].present=0;
      }
      cam->format7_info.edit_mode=-1;
    }
  }  
}

void
GetFormat7ModeInfo(camera_t* cam, int mode_id) 
{
  Format7ModeInfo_t *mode;
  mode=&cam->format7_info.mode[mode_id-MODE_FORMAT7_MIN];

  if (mode->present>0) { // check for mode presence before query
    if (dc1394_query_format7_max_image_size(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->max_size_x,&mode->max_size_y)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 max image size");
    if (dc1394_query_format7_unit_size(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->unit_size_x,&mode->unit_size_y)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 unit size");
    // quick hack to keep size/position even. If pos/size is ODD, strange color/distorsions occur on some cams
    // (e.g. Basler cams). This will have to really fixed later.
    // REM: this is fixed by using the unit_position:
    // fprintf(stderr,"Using pos units = %d %d\n",info->mode[i].step_pos_x,info->mode[i].step_pos_y);
    if (dc1394_query_format7_unit_position(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->unit_pos_x,&mode->unit_pos_y)!=DC1394_SUCCESS) {
      MainError("Got a problem querying format7 unit position");
      mode->unit_pos_x=0;
      mode->unit_pos_y=0;
    }
    if (dc1394_query_format7_image_position(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->pos_x,&mode->pos_y)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 image position");
    if (dc1394_query_format7_image_size(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->size_x,&mode->size_y)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 image size");
    if (dc1394_query_format7_byte_per_packet(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->bpp)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 bytes per packet");
    if (mode->bpp==0) {
      // sometimes a camera will not set the bpp register until a valid image size has been set after boot. If BPP is zero, we therefor
      // try again after setting the image size to the maximum size.
      MainError("Camera reported a BPP of ZERO. Trying to set maximum size to correct this.");
      if (dc1394_set_format7_image_position(cam->camera_info.handle,cam->camera_info.id,mode_id,0,0)!=DC1394_SUCCESS)
	MainError("Got a problem setting format7 image position");
      if (dc1394_set_format7_image_size(cam->camera_info.handle,cam->camera_info.id,mode_id,mode->max_size_x,mode->max_size_y)!=DC1394_SUCCESS)
	MainError("Got a problem setting format7 image size");
      // maybe we should also force a color coding here.
      if (dc1394_query_format7_byte_per_packet(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->bpp)!=DC1394_SUCCESS)
	MainError("Got a problem querying format7 bytes per packet");
      if (mode->bpp==0) {
	MainError("    BPP still zero. Giving up.");
      }
    }

    if (dc1394_query_format7_packet_para(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->min_bpp,&mode->max_bpp)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 packet parameters");
    if (dc1394_query_format7_pixel_number(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->pixnum)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 pixel number");
    if (dc1394_query_format7_total_bytes(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->total_bytes)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 total bytes per frame");
    if (dc1394_query_format7_color_coding_id(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->color_coding_id)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 color coding ID");
    if (dc1394_query_format7_color_coding(cam->camera_info.handle,cam->camera_info.id,mode_id,&mode->color_coding)!=DC1394_SUCCESS)
      MainError("Got a problem querying format7 color coding");
    //fprintf(stderr,"%d\n",(int)mode->total_bytes);
  }
}
void
SwitchToNearestFPS(quadlet_t compat, int current) {
  int i;
  dc1394bool_t cont=DC1394_TRUE;
  int new_framerate=-1;
  char *temp;

  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  current=current-FRAMERATE_MIN;

  for (i=0;i<=((NUM_FRAMERATES>>1)+1);i++) { // search radius is num_framerates/2 +1 for safe rounding
    if ( (compat&(0x1<<(31-(current+i)))) && cont) {
      new_framerate=current+i+FRAMERATE_MIN;
      cont=DC1394_FALSE;
    }
    if ( (compat&(0x1<<(31-(current-i)))) && cont) {
      new_framerate=current-i+FRAMERATE_MIN;
      cont=DC1394_FALSE;
    }
  }

  if (new_framerate!=current) {
    sprintf(temp,"Invalid framerate. Updating to nearest: %s",fps_label_list[new_framerate-FRAMERATE_MIN]);
    MainStatus(temp);
    if (dc1394_set_video_framerate(camera->camera_info.handle,camera->camera_info.id,new_framerate)!=DC1394_SUCCESS) {
      MainError("Cannot set video framerate");
    }
    else {
      camera->misc_info.framerate=new_framerate;
    }
  }

  free(temp);
}

void
ChangeModeAndFormat         (GtkMenuItem     *menuitem,
			     gpointer         user_data)
{
  int state;
  int format;
  quadlet_t value;
  int mode;
  
  mode=(int)user_data;

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
 
  // check consistancy of framerate:
  if (camera->misc_info.format!=FORMAT_SCALABLE_IMAGE_SIZE) {
    if (dc1394_query_supported_framerates(camera->camera_info.handle, camera->camera_info.id, format, mode, &value)!=DC1394_SUCCESS)
      MainError("Could not read supported framerates");
    else {
      if ((value & (0x1<<(31-(camera->misc_info.framerate-FRAMERATE_MIN))))==0) {
	// the current framerate is not OK for the new mode/format. Switch to nearest framerate
	SwitchToNearestFPS(value,camera->misc_info.framerate);
      }
    }
  }

  IsoFlowResume(&state);

  // REPROBE EVERYTHING
  if (dc1394_get_camera_info(camera->camera_info.handle,camera->camera_info.id, &camera->camera_info)!=DC1394_SUCCESS)
    MainError("Could not get camera basic information!");
  if (dc1394_get_camera_misc_info(camera->camera_info.handle,camera->camera_info.id, &camera->misc_info)!=DC1394_SUCCESS)
    MainError("Could not get camera misc information!");
  if (dc1394_get_camera_feature_set(camera->camera_info.handle,camera->camera_info.id, &camera->feature_set)!=DC1394_SUCCESS)
    MainError("Could not get camera feature information!");

  if (format==FORMAT_SCALABLE_IMAGE_SIZE) {
    GetFormat7Capabilities(camera);
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
  *state=(GetService(camera, SERVICE_ISO)!=NULL);
  if (*state!=0) {
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window,"service_iso"),FALSE);
  }
}

void IsoFlowResume(int *state)
{
  int was_on;
  int timeout;

  was_on=camera->misc_info.is_iso_on;
  if (was_on>0) { // restart if it was 'on' before the changes
    usleep(DELAY); // necessary to avoid desynchronized ISO flow.
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
	MainError("ISO could not be restarted. Trying again for 5 seconds");
	timeout=0;
	while ((!camera->misc_info.is_iso_on)&&(timeout<5000)) {
	  usleep(DELAY);
	  timeout+=DELAY/1000;
	  if (dc1394_start_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
	    // ... (if not done, restarting is no more possible)
	    MainError("Could not start ISO transmission");
	  else {
	    if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id,&camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
	      MainError("Could not get ISO status");
	  }
	}
	if (!camera->misc_info.is_iso_on)
	  MainError("Can't start ISO, giving up...");
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


  // format7:
  ctxt.format7_imagebytes_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"format7_imagebytes"),"");
  ctxt.format7_totalbytes_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"format7_totalbytes"),"");
  ctxt.format7_padding_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"format7_padding"),"");
  ctxt.format7_imagepixels_ctxt=gtk_statusbar_get_context_id( (GtkStatusbar*) lookup_widget(main_window,"format7_imagepixels"),"");

  ctxt.format7_imagebytes_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"format7_imagebytes"), ctxt.format7_imagebytes_ctxt, "");
  ctxt.format7_imagepixels_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"format7_imagepixels"), ctxt.format7_imagepixels_ctxt, "");
  ctxt.format7_totalbytes_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"format7_totalbytes"), ctxt.format7_totalbytes_ctxt, "");
  ctxt.format7_padding_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"format7_padding"), ctxt.format7_padding_ctxt, "");
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
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  sprintf(temp,"ERROR: %s\n",string);
  if (main_window !=NULL) {
    gtk_text_insert((GtkText*)lookup_widget(main_window,"main_status"), NULL,NULL,NULL,temp,-1);
  }
  free(temp);
}

void MainStatus(const char *string)
{
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(temp,"%s\n",string);
  if (main_window !=NULL) {
    gtk_text_insert((GtkText*)lookup_widget(main_window,"main_status"), NULL,NULL,NULL,temp,-1);
  }
  free(temp);
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
  char *stemp;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

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
  free(stemp);
}

void
SetAbsoluteControl(int feature, int power)
{
  if (dc1394_absolute_setting_on_off(camera->camera_info.handle, camera->camera_info.id, feature, power)!=DC1394_SUCCESS)
    MainError("Could not toggle absolute setting control\n");
  else {
    camera->feature_set.feature[feature-FEATURE_MIN].abs_control=power;
    if (power>0) {
      GetAbsValue(feature);
    }
    else {
      UpdateRange(feature);
    }
  }
}


void
SetAbsValue(int feature)
{
  char *stemp, *string;
  char *stringp;
  float value;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));
  string=(char*)malloc(STRING_SIZE*sizeof(char));

 
  sprintf(stemp,"feature_%d_abs_entry",feature);
  stringp=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,stemp)));
  value=atof(stringp);
  if (dc1394_set_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, value)!=DC1394_SUCCESS) {
    MainError("Can't set absolute value!");
  }
  else {
    if (dc1394_query_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, &value)!=DC1394_SUCCESS) {
      MainError("Can't get absolute value!");
    }
    else {
      sprintf(string,"%.8f",value);
      gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window,stemp)),string);
    }
  }
  free(stemp);
  free(string);
}

void
GetAbsValue(int feature)
{
  char *stemp, *string;
  float value;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));
  string=(char*)malloc(STRING_SIZE*sizeof(char));
 
  
  if (dc1394_query_absolute_feature_value(camera->camera_info.handle, camera->camera_info.id, feature, &value)!=DC1394_SUCCESS) {
    MainError("Can't get absolute value!");
  }
  else {
    sprintf(string,"%.8f",value);
    sprintf(stemp,"feature_%d_abs_entry",feature);
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window,stemp)),string);
  }
  free(stemp);
  free(string);
}

/*
  The timeout/bus_reset handling technique is 'strongly inspired' by the code found in
  GScanbus by Andreas Micklei.
*/

int
bus_reset_handler(raw1394handle_t handle, unsigned int generation) {

  BusInfo_t bi; // WHY NOT USE THE BUS_INFO GLOBAL VARIABLE HERE???
  int i, ic, icfound, channel, speed;
  int port;
  camera_t *camera_ptr, *cp2;
  camera_t* new_camera;
  int index;
  dc1394bool_t iso_status;
  dc1394_camerainfo camera_info;
  unsigned long long int new_guids[128];

  bi.handles=NULL;
  bi.port_camera_num=NULL;
  bi.camera_nodes=NULL;
  MainStatus("Bus reset detected");

  gtk_widget_set_sensitive(main_window,FALSE);

  usleep(DELAY); // sleep some time (50ms) to allow the cam to warm-up/boot

  raw1394_update_generation(handle, generation);
  // Now we have to deal with this bus reset...

  // get camera nodes:
  GetCameraNodes(&bi);
  /*
  cp2=cameras;
  while(cp2!=NULL) {
    fprintf(stderr,"Channel %d used\n",cp2->misc_info.iso_channel);
    cp2=cp2->next;
  }
  */
  // ADD NEW CAMERAS AND UPDATE PREVIOUS ONES ---------------------------------

  // try to match the GUID with previous cameras
  for (port=0;port<bi.port_num;port++) {
    if (bi.handles[port]!=0) {
      for (i=0;i<bi.port_camera_num[port];i++) {
	if (dc1394_get_camera_info(bi.handles[port], bi.camera_nodes[port][i], &camera_info)!=DC1394_SUCCESS)
	  MainError("Can't get camera basic information in bus-reset handler");
	//fprintf(stderr, " current GUID: 0x%llx\n", camera_info.euid_64);

	// was the current GUID already there?
	camera_ptr=cameras;
	while(camera_ptr!=NULL) {
	  if (camera_ptr->camera_info.euid_64==camera_info.euid_64) { // yes, the camera was there
	    //fprintf(stderr,"  camera was already there, updating...\n");
	    if (dc1394_get_camera_info(bi.handles[port], bi.camera_nodes[port][i], &camera_ptr->camera_info)!=DC1394_SUCCESS)
	      MainError("Could not update camera basic information in bus reset handler");
	    // If ISO service is on, stop it and restart it.
	    /*if (GetService(camera_ptr,SERVICE_ISO)!=NULL) {
	      fprintf(stderr,"  Restarting ISO service\n");
	      IsoStopThread(camera_ptr);
	      fprintf(stderr,"   stop\n");
	      usleep(DELAY);
	      IsoStartThread(camera_ptr);
	      fprintf(stderr,"   start\n");
	      }*/
	    break;
	  }
	  camera_ptr=camera_ptr->next;
	}
	if (camera_ptr==NULL) { // the camera is new
	  //fprintf(stderr,"  A new camera was added\n");
	  new_camera=NewCamera();
	  GetCameraData(port, bi.camera_nodes[port][i], new_camera);

	  // set ISO channel for this camera
	  ic=0;
	  icfound=0;
	  while(icfound!=1) {
	    //fprintf(stderr,"    Trying channel %d...\n",ic);
	    cp2=cameras;
	    while(cp2!=NULL) {
	      if (cp2->misc_info.iso_channel==ic) {
		//fprintf(stderr,"    Found a cam with channel %d\n",channel);
		break;
	      }
	      cp2=cp2->next;
	    }
	    if (cp2==NULL)
	      icfound=1;
	    else
	      ic++;
	  }
	  if (dc1394_get_iso_channel_and_speed(new_camera->camera_info.handle, new_camera->camera_info.id,
					       &channel, &speed)!=DC1394_SUCCESS)
	    MainError("Can't get iso channel and speed");
	  //fprintf(stderr,"   Channel was %d\n",channel);
	  if (dc1394_set_iso_channel_and_speed(new_camera->camera_info.handle, new_camera->camera_info.id,
					       ic, speed)!=DC1394_SUCCESS)
	    MainError("Can't set iso channel and speed");
	  //fprintf(stderr,"   Channel set to %d\n",ic);
	  if (cameras==NULL) {
	    AppendCamera(new_camera);
	    SetCurrentCamera(new_camera->camera_info.euid_64);
	  }
	  else {
	    AppendCamera(new_camera);
	  }
	}
      }
    }
  }

  // CLEAR REMOVED CAMERAS -----------------------------

  index=0;
  //fprintf(stderr,"Getting new GUIDs\n");
  // get all new guids
  for (port=0;port<bi.port_num;port++) {
    if (bi.handles[port]!=0) {
      for (i=0;i<bi.port_camera_num[port];i++) {
	if (dc1394_get_camera_info(bi.handles[port], bi.camera_nodes[port][i], &camera_info)!=DC1394_SUCCESS)
	  MainError("Can't get camera basic information in bus-reset handler");
	new_guids[index]=camera_info.euid_64;
	index++;
      }
    }
  }
  // look if there is a camera that disappeared from the camera_t struct
  camera_ptr=cameras;
  while (camera_ptr!=NULL) {
    for (i=0;i<index;i++) {
      if (camera_ptr->camera_info.euid_64==new_guids[i])
	break;
    }
    if (camera_ptr->camera_info.euid_64!=new_guids[i]) { // the camera "camera_ptr" was unplugged
      //fprintf(stderr,"found a camera to remove\n");
      if (camera->camera_info.euid_64==camera_ptr->camera_info.euid_64) {
	//fprintf(stderr," The current camera was unplugged\n");
	if (bi.camera_num==0) { // the only camera was removed. Close GUI and revert to camera wait prompt
	  //fprintf(stderr," ... and it was the only camera!\n");
	  waiting_camera_window=create_waiting_camera_window();
	  gtk_widget_show(waiting_camera_window);

	  // delete structs:
	  RemoveCamera(camera_ptr->camera_info.euid_64);
	  cameras=NULL;
	  camera=NULL;
	}
	else {
	  //fprintf(stderr,"  Selecting the first non-removed camera as current camera\n");
	  if (cameras->camera_info.euid_64==camera_ptr->camera_info.euid_64) { // is the first camera the one to be removed?
	    // use second cam as current cam
	    SetCurrentCamera(cameras->next->camera_info.euid_64);
	  }
	  else {
	    // use first cam as current cam
	    SetCurrentCamera(cameras->camera_info.euid_64);
	  }
	  // close and remove dead camera
	  RemoveCamera(camera_ptr->camera_info.euid_64);
	}
	//fprintf(stderr," removed dead camera\n");
      } // end if we are deleting the current camera
      else { // we delete another camera. This is easy.
	RemoveCamera(camera_ptr->camera_info.euid_64);
      }
      // rescan from the beginning.
      camera_ptr=cameras;
    }
    else {
      camera_ptr=camera_ptr->next;
    }
  }
  //fprintf(stderr,"Removed all dead camera structs\n");

  // restart ISO if necessary
  cp2=cameras;
  while(cp2!=NULL) {
    if (dc1394_get_iso_status(cp2->camera_info.handle,cp2->camera_info.id,&iso_status)!=DC1394_SUCCESS) {
      MainError("Could not read ISO status");
    }
    else {
      //fprintf(stderr,"iso is %d and should be %d\n", iso_status,cp2->misc_info.is_iso_on);
      if ((cp2->misc_info.is_iso_on==DC1394_TRUE)&&(iso_status==DC1394_FALSE)) {
	if (dc1394_start_iso_transmission(cp2->camera_info.handle,cp2->camera_info.id)!=DC1394_SUCCESS) {
	  MainError("Could start ISO");
	}
	usleep(DELAY);
      }
    }
    cp2=cp2->next;
  }

  if (bi.camera_num>0) {
    //fprintf(stderr,"build/refresh GUI\n");
    if (waiting_camera_window!=NULL) {
      gtk_widget_destroy(GTK_WIDGET(waiting_camera_window));
      waiting_camera_window=NULL;
      //fprintf(stderr," destroyed win\n");
    }

#ifdef HAVE_SDLLIB
    watchthread_info.draw=0;
    watchthread_info.mouse_down=0;
    watchthread_info.crop=0;
#endif
    /*
    if (new_camera!=NULL)
      fprintf(stderr," handle: 0x%x\n",new_camera->camera_info.handle);
    
    if (camera!=NULL)
      fprintf(stderr," handle: 0x%x\n",camera->camera_info.handle);
    fprintf(stderr,"camera: 0x%x\n",camera);
    */
    //fprintf(stderr,"Want to display: %d\n",camera->want_to_display);
    if (camera!=NULL) {
      if (camera->want_to_display>0)
	DisplayStartThread(camera);

      BuildAllWindows();
      //fprintf(stderr,"finished building GUI\n");
      UpdateAllWindows();
      //fprintf(stderr,"finished updating GUI\n");

      gtk_widget_set_sensitive(main_window,TRUE);
    }
  }

  GrabSelfIds(bi.handles, bi.port_num);

  /*
  fprintf(stderr,"Reseting ISO channels\n");
  // re-set ISO channels.
  SetChannels();

  fprintf(stderr,"Restarting ISO\n");
  // Restart all ISO threads
  camera_ptr=cameras;
  while (camera_ptr!=NULL) {
    if (GetService(camera_ptr,SERVICE_ISO)!=NULL) {
      IsoStopThread(camera_ptr);
      usleep(DELAY);
      IsoStartThread(camera_ptr);
    }
    camera_ptr=camera_ptr->next;
  }
  */
  free(bi.handles);
  free(bi.port_camera_num);
  free(bi.camera_nodes);

  //fprintf(stderr,"resumed fps display\n");

  //fprintf(stderr,"Finished handling bus reset\n");

  return(1);
}

int
main_timeout_handler(gpointer* port_num) {

  int i;
  //int ports=(int)port_num;
  quadlet_t quadlet;

  // the main timeout performs tasks every ms. In order to have less repeated tasks
  // the main_timeout_ticker can be consulted.
  main_timeout_ticker=(main_timeout_ticker+10)%1000;

  // --------------------------------------------------------------------------------------
  // cancel display thread if asked by the SDL/WM
  // We must do this here because it is not allowed to call a GTK function from a thread. At least if we do
  // so the program progressively breaks with strange GUI behaviour/look.
  if (!(main_timeout_ticker%100)) { // every 100ms
    if (WM_cancel_display>0) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_display")), FALSE);
      WM_cancel_display=0;
    }
    //fprintf(stderr,"check display cancel\n");
  }

  // --------------------------------------------------------------------------------------
  // performs a dummy read on all handles to detect bus resets
  if (!(main_timeout_ticker%1000)) { // every second
    for (i=0;i<businfo->port_num;i++) {
      //fprintf(stderr,"bus reset detection for port %d\n",i);
      cooked1394_read(businfo->handles[i], 0xffc0 | raw1394_get_local_id(businfo->handles[i]),
		      CSR_REGISTER_BASE + CSR_CYCLE_TIME, 4, (quadlet_t *) &quadlet);
    }
    //fprintf(stderr,"dummy read\n");
  }
  // --------------------------------------------------------------------------------------
  // update the bandwidth estimtation
  if (!(main_timeout_ticker%1000)) { // every second
    UpdateBandwidthFrame();
  }
#ifdef HAVE_SDLLIB
  // --------------------------------------------------------------------------------------
  // update cursor information
  if (!(main_timeout_ticker%100)) { // every 100ms
    if (cursor_info.update_req>0) {
      UpdateCursorFrame();
      cursor_info.update_req=0;
    }
  }
#endif
  return(1);
}


void
SetFormat7Crop(int sx, int sy, int px, int py, int mode) {
	
  int state;
  Format7ModeInfo_t *info;
  GtkAdjustment *adjsx, *adjsy, *adjpx, *adjpy;

  info=&camera->format7_info.mode[mode-MODE_FORMAT7_MIN];

  adjpx=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_hposition_scale")));
  adjpy=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_vposition_scale")));
  adjsx=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_hsize_scale")));
  adjsy=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_vsize_scale")));
  
  // do something if we were called by a first generation signal:
  if ((gtk_signal_n_emissions_by_name(GTK_OBJECT (adjpx), "changed")==0)&&
      (gtk_signal_n_emissions_by_name(GTK_OBJECT (adjpy), "changed")==0)&&
      (gtk_signal_n_emissions_by_name(GTK_OBJECT (adjsx), "changed")==0)&&
      (gtk_signal_n_emissions_by_name(GTK_OBJECT (adjsy), "changed")==0)) {

    if (mode==camera->misc_info.mode) {
      IsoFlowCheck(&state);
    }
    
    // the order in which we apply the F7 changes is important.
    // example: from size=128x128, pos=128x128, we can't go to size=1280x1024 by just changing the size.
    // We need to set the position to 0x0 first.
    //fprintf(stderr,"Setting format7 to pos=[%d %d], size=[%d %d]\n",px,py,sx,sy);
    if (dc1394_set_format7_image_position(camera->camera_info.handle,camera->camera_info.id, mode, 0, 0)!=DC1394_SUCCESS)
      MainError("Could not set Format7 image position to zero");
    if ((dc1394_set_format7_image_size(camera->camera_info.handle,camera->camera_info.id, mode, sx, sy)!=DC1394_SUCCESS)||
	(dc1394_set_format7_image_position(camera->camera_info.handle,camera->camera_info.id, mode, px, py)!=DC1394_SUCCESS))
      MainError("Could not set Format7 image size and position");
    else {
      info->size_x=sx;
      info->size_y=sy;
      info->pos_x=px;
      info->pos_y=py;
    }
    
    // tell the ranges to change their settings
    adjpx->upper=info->max_size_x-sx;
    adjpx->value=px;
    gtk_signal_emit_by_name(GTK_OBJECT (adjpx), "changed");
    
    adjpy->upper=info->max_size_y-sy;
    adjpy->value=py;
    gtk_signal_emit_by_name(GTK_OBJECT (adjpy), "changed");
    
    adjsx->upper=info->max_size_x-px;
    adjsx->value=sx;
    gtk_signal_emit_by_name(GTK_OBJECT (adjsx), "changed");
    
    adjsy->upper=info->max_size_y-py;
    adjsy->value=sy;
    gtk_signal_emit_by_name(GTK_OBJECT (adjsy), "changed");
    
    usleep(DELAY);

    if (mode==camera->misc_info.mode) {
      IsoFlowResume(&state);
    }
  }
} 

int
NearestValue(int value, int step, int min, int max) {

  int low, high;

  if (((max-min)%step) !=0) {
    MainError("Stange values entered in NearestValue...");
  }

  low=value-(value%step);
  high=value-(value%step)+step;
  if (low<min)
    low=min;
  if (high>max)
    high=max;

  if (abs(low-value)<abs(high-value))
    return low;
  else
    return high;
}

void
window_set_icon(GtkWidget* window) {

  // This function is 'strongly inspired' by the related code found in gqview.

#ifdef HAVE_GDK_PIXBUF
  GdkPixbuf *pb;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  pb = gdk_pixbuf_new_from_xpm_data((const gchar**)coriander_icon_xpm);
  gdk_pixbuf_render_pixmap_and_mask(pb, &pixmap, &mask, 128);
  gdk_pixbuf_unref(pb);
  gdk_window_set_icon(window->window, NULL, pixmap, mask);
#endif

}

/* The following function is a strip-down version of the program 'xvinfo' of the XFree86 project. */
void
GetXvInfo(xvinfo_t *xvinfo) {

#ifdef HAVE_XV

  unsigned int ver, rev, eventB, reqB, errorB; 
  int i, j, n; 

  xvinfo->max_height=-1;
  xvinfo->max_width=-1;

  if(!(xvinfo->dpy = XOpenDisplay(NULL))) {
    MainError("Unable to open display");
    return;
  }
  
  if((Success != XvQueryExtension(xvinfo->dpy, &ver, &rev, &reqB, &eventB, &errorB))) {
    MainError("xvinfo: No X-Video Extension");
    return;
  }

  i=0; // 0: we use only the first screen
  XvQueryAdaptors(xvinfo->dpy, RootWindow(xvinfo->dpy, i), &xvinfo->nadaptors, &xvinfo->ainfo);
  if( !xvinfo->nadaptors || !xvinfo->ainfo ){        
    MainError( "xvinfo: No adpators present\n" );        
    return;
  }
  j=0; // 0: we use only the first adaptor
  XvQueryEncodings(xvinfo->dpy, xvinfo->ainfo[j].base_id, &xvinfo->nencode, &xvinfo->encodings);

  if(xvinfo->encodings && xvinfo->nencode) {
    xvinfo->ImageEncodings = 0;
    
    for(n = 0; n < xvinfo->nencode; n++) {
      if(!strcmp(xvinfo->encodings[n].name, "XV_IMAGE"))
	xvinfo->ImageEncodings++;
    }

    if(xvinfo->ImageEncodings && (xvinfo->ainfo[j].type & XvImageMask)) {
      //char imageName[5] = {0, 0, 0, 0, 0};
      
      for(n = 0; n < xvinfo->nencode; n++) {
	if(!strcmp(xvinfo->encodings[n].name, "XV_IMAGE")) {
	  //fprintf(stdout, "maximum XvImage size: %li x %li\n", xvinfo->encodings[n].width, xvinfo->encodings[n].height);
	  xvinfo->max_height=(int)xvinfo->encodings[n].height;
	  xvinfo->max_width=(int)xvinfo->encodings[n].width;
	  break;
	}
      }
      /*
      formats = XvListImageFormats(dpy, ainfo[j].base_id, &numImages);   
      fprintf(stdout, "    Number of image formats: %i\n", numImages);

      for(n = 0; n < numImages; n++) {
	memcpy(imageName, &(formats[n].id), 4);
	fprintf(stdout, "      id: 0x%x", formats[n].id);
	if(isprint(imageName[0]) && isprint(imageName[1]) && isprint(imageName[2]) && isprint(imageName[3])) {
	  fprintf(stdout, " (%s)\n", imageName);
	} else {
	  fprintf(stdout, "\n");
	}
	fprintf(stdout, "        bits per pixel: %i\n", formats[n].bits_per_pixel);
	fprintf(stdout, "        number of planes: %i\n", formats[n].num_planes);
	fprintf(stdout, "        type: %s (%s)\n",
		(formats[n].type == XvRGB) ? "RGB" : "YUV",
		(formats[n].format == XvPacked) ? "packed" : "planar");
	if(formats[n].type == XvRGB) {
	  fprintf(stdout, "        depth: %i\n", formats[n].depth);
	  fprintf(stdout, "        red, green, blue masks: 0x%x, 0x%x, 0x%x\n", 
		  formats[n].red_mask,
		  formats[n].green_mask,
		  formats[n].blue_mask);
	} 
      }
      if(formats) XFree(formats);
      */
    }
      
    XvFreeEncodingInfo(xvinfo->encodings);
  }

  XvFreeAdaptorInfo(xvinfo->ainfo);

#else
  xvinfo->max_height=-1;
  xvinfo->max_width=-1;
#endif

  //fprintf(stderr,"%d %d\n", xvinfo->max_height, xvinfo->max_width);
}

