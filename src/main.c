/*
 *                     C  O  R  I  A  N  D  E  R
 *
 *            -- The IEEE-1394 Digital Camera controller --
 *
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
 *               Ftp and conversions by Dan Dennedy <ddennedy@coolsite.net>
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
#include "interface.h"
#include "support.h"
#include "callbacks.h"
#include "build_windows.h"
#include "update_windows.h"
#include "definitions.h"
#include "tools.h"
#include "watch_thread.h"
#include "preferences.h"
#include "thread_base.h"
#include "raw1394support.h"
#include <libdc1394/dc1394_control.h>
#include <libraw1394/raw1394.h>

GtkWidget *commander_window;
GtkWidget *absolute_settings_window;
GtkWidget *about_window;
GtkWidget *help_window;
GtkWidget *format7_window;
GtkWidget *preferences_window;


dc1394_camerainfo *camera;
dc1394_camerainfo *cameras;
dc1394_feature_set *feature_set;
dc1394_feature_set *feature_sets;
dc1394_miscinfo *misc_info;
dc1394_miscinfo *misc_infos;
chain_t **image_pipes;
chain_t *image_pipe;
Format7Info *format7_infos;
Format7Info *format7_info;
UIInfo *uiinfos;
UIInfo *uiinfo;
StatusInfo *statusinfos;
StatusInfo *statusinfo;
CtxtInfo ctxt;
SelfIdPacket_t *selfid;
SelfIdPacket_t *selfids;
PrefsInfo preferences;
int silent_ui_update;
int camera_num;
int current_camera;
whitebal_data_t *whitebal_data;

#ifdef HAVE_SDLLIB
  watchthread_info_t watchthread_info;
#endif

int
main (int argc, char *argv[])
{
  int err, i, cam;
  nodeid_t **camera_nodes=NULL; 
  raw1394handle_t *handles=NULL;
  raw1394handle_t tmp_handle;
  int port;
  int index;
  int *port_camera_num=NULL;
  int portmax=0;
  int card_found;
  //float tmp;
  
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  gnome_init ("coriander", VERSION, argc, argv);

  whitebal_data=(whitebal_data_t*)malloc(sizeof(whitebal_data));
  card_found=0;
  tmp_handle=raw1394_new_handle();
  if (tmp_handle!=NULL)
    {
      portmax=raw1394_get_port_info(tmp_handle, NULL, 0);
      raw1394_destroy_handle(tmp_handle);

      camera_nodes=(nodeid_t**)malloc(portmax*sizeof(nodeid_t*));
      port_camera_num=(int*)malloc(portmax*sizeof(int));
      handles=(raw1394handle_t *)malloc(portmax*sizeof(raw1394handle_t));
  
      for (port=0;port<portmax;port++)
	{
	  // get a handle to the current interface card
	  handles[port]=dc1394_create_handle(port);
	  if (handles[port]!=0) // if the card is present
	    {
	      card_found=1;
	      // probe the IEEE1394 bus for DC camera:
	      camera_nodes[port]=dc1394_get_camera_nodes(handles[port], &port_camera_num[port], 0); // 0 not to show the cams.
	      camera_num+=port_camera_num[port];
	    }
	}
    }
  if (card_found==0)
    {
      gtk_widget_show(create_no_handle_window());
      gtk_main ();
    }
  else
    {
      if (camera_num<1)
	{
	  gtk_widget_show(create_no_camera_window());
	  gtk_main ();
	}
      else// we have at least one camera on one interface card.
	{
	  // allocate memory space for all camera infos & download all infos:
	  cameras=(dc1394_camerainfo*)calloc(camera_num,sizeof(dc1394_camerainfo));
	  feature_sets=(dc1394_feature_set*)calloc(camera_num,sizeof(dc1394_feature_set));
	  misc_infos=(dc1394_miscinfo*)calloc(camera_num,sizeof(dc1394_miscinfo));
	  image_pipes=(chain_t**)calloc(camera_num,sizeof(chain_t*));
	  format7_infos=(Format7Info*)calloc(camera_num,sizeof(Format7Info));
	  uiinfos=(UIInfo*)calloc(camera_num,sizeof(UIInfo));
	  selfids=(SelfIdPacket_t*)calloc(camera_num,sizeof(SelfIdPacket_t));
	  preferences.camera_names=(char **)calloc(camera_num,sizeof(char*));

	  // get camera infos and serialize the port info for each camera
	  index=0;
	  for (port=0;port<portmax;port++)
	    {
	      if (handles[port]!=0)
		for (cam=0;cam<port_camera_num[port];cam++)
		  {
		    err=dc1394_get_camera_info(handles[port], camera_nodes[port][cam], &cameras[index]);
		    if (err<0) MainError("Could not get camera basic information!");
		    err=dc1394_get_camera_misc_info(cameras[index].handle, cameras[index].id, &misc_infos[index]);
		    if (err<0) MainError("Could not get camera misc information!");
		    err=dc1394_get_camera_feature_set(cameras[index].handle, cameras[index].id, &feature_sets[index]);
		    if (err<0) MainError("Could not get camera feature information!");
		    //dc1394_print_feature_set(&feature_sets[index]);
		    //dc1394_absolute_setting_on_off(cameras[index].handle, cameras[index].id, FEATURE_SHUTTER,0);
		    //feature_sets[index].feature[FEATURE_SHUTTER-FEATURE_MIN].abs_control=0;
		    //tmp=.00000001;
		    //dc1394_set_absolute_feature_value(cameras[index].handle, cameras[index].id, FEATURE_SHUTTER, &tmp);
		    //dc1394_query_absolute_feature_value(cameras[index].handle, cameras[index].id, FEATURE_SHUTTER, &tmp);
		    //fprintf(stderr,"returned value: %3.8f\n",tmp);
		    GetFormat7Capabilities(cameras[index].handle, cameras[index].id, &format7_infos[index]);
		    image_pipes[index]=NULL;
		    uiinfos[index].test_pattern=0;
		    uiinfos[index].want_to_display=0;
		    uiinfos[index].bayer=NO_BAYER_DECODING;
		    uiinfos[index].stereo=NO_STEREO_DECODING;
		    uiinfos[index].bpp=8;
		    index++;
		  }
	    }

	  GrabSelfIds(handles, portmax);
	  silent_ui_update=0;
	  SetChannels();
	  
	  // current camera is the first camera:
	  SelectCamera(0);
	  
	  // Create the permanent control windows.
	  // (note BTW that other windows like 'file_selector' are created
	  //  and destroyed on purpose while the following windows always exist.)

	  g_thread_init(NULL);
	  
	  preferences_window= create_preferences_window();
	  commander_window = create_commander_window();
	  format7_window = create_format7_window();
	  absolute_settings_window = create_absolute_settings_window();

	  // Setup the GUI in accordance with the camera capabilities
	  GetContextStatus();
	  BuildAllWindows();
	  UpdateAllWindows();
#ifdef HAVE_SDLLIB
	  WatchStartThread(&watchthread_info);
#endif
	  MainStatus("Welcome to Coriander...");
	  gtk_widget_show (commander_window); // this is the only window shown at boot-time

	  gdk_threads_enter();
	  gtk_main();
	  gdk_threads_leave();
	  
	  // clean all threads for all cams:
	  for (i=0;i<camera_num;i++)
	    {
	      SelectCamera(i);
	      CleanThreads(CLEAN_MODE_NO_UI_UPDATE);
	    }
#ifdef HAVE_SDLLIB
	  WatchStopThread(&watchthread_info);
#endif
	  
	  free(cameras);
	  free(feature_sets);
	  free(misc_infos);
	  free(image_pipes);
	  free(format7_infos);
	  free(uiinfos);
	  free(selfids);

	  for (i=0;i<portmax;i++)
	    {
	      if (handles[i]!=0)
		raw1394_destroy_handle(handles[i]);
	      free(camera_nodes[i]);
	    }

	  free(camera_nodes);
	  free(handles);
	  free(port_camera_num);
	  free(preferences.camera_names);
	  
	} // end of if no handle check

    } // end of if no camera check
  free(whitebal_data);
  
  return 0;
}
