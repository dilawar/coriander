/*
 *                     C  O  R  I  A  N  D  E  R
 *
 *            -- The IEEE-1394 Digital Camera controller --
 *
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
#include "raw1394support.h"
#include "interface.h"
#include "support.h"
#include "definitions.h"
#include "camera.h"
#include "thread_base.h"
#include "build_windows.h"
#include "update_windows.h"
#include "tools.h"
#include "watch_thread.h"
#include "preferences.h"
#include "thread_iso.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "thread_display.h"
#include "SDLEvent.h"

GtkWidget *main_window;
GtkWidget *absolute_settings_window;
GtkWidget *about_window;
GtkWidget *help_window;
GtkWidget *preferences_window;
CtxtInfo ctxt;
PrefsInfo preferences;
int silent_ui_update;
camera_t* camera;
camera_t* cameras;

raw1394handle_t *handles;
//whitebal_data_t *whitebal_data;

unsigned int main_timeout_ticker;
unsigned int WM_cancel_display;
cursor_info_t cursor_info;

#ifdef HAVE_SDLLIB
  watchthread_info_t watchthread_info;
#endif

int
main (int argc, char *argv[])
{
  int i, cam;
  nodeid_t **camera_nodes=NULL; 
  raw1394handle_t tmp_handle;
  int port;
  int camera_num;
  int index;
  int *port_camera_num=NULL;
  int portmax=0;
  int card_found;
  int main_timeout;
  camera_t* camera_ptr;
  
  main_timeout_ticker=0;
  WM_cancel_display=0;
  handles=NULL;
  camera_num=0;
  cameras=NULL;

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  gnome_init ("coriander", VERSION, argc, argv);

  //whitebal_data=(whitebal_data_t*)malloc(sizeof(whitebal_data));
  card_found=0;
  tmp_handle=raw1394_new_handle();
  if (tmp_handle!=NULL) {
    portmax=raw1394_get_port_info(tmp_handle, NULL, 0);
    raw1394_destroy_handle(tmp_handle);
    
    camera_nodes=(nodeid_t**)malloc(portmax*sizeof(nodeid_t*));
    port_camera_num=(int*)malloc(portmax*sizeof(int));
    handles=(raw1394handle_t *)malloc(portmax*sizeof(raw1394handle_t));
    
    for (port=0;port<portmax;port++) {
      // get a handle to the current interface card
      handles[port]=dc1394_create_handle(port);
      if (handles[port]!=0) { // if the card is present
	card_found=1;
	// probe the IEEE1394 bus for DC camera:
	camera_nodes[port]=dc1394_get_camera_nodes(handles[port], &port_camera_num[port], 0); // 0 not to show the cams.
	camera_num+=port_camera_num[port];
      }
    }
  }
  if (card_found==0) {
    gtk_widget_show(create_no_handle_window());
    gtk_main ();
  }
  else {
    if (camera_num<1) {
      gtk_widget_show(create_no_camera_window());
      gtk_main ();
    }
    else { // we have at least one camera on one interface card.
      // get camera infos and serialize the port info for each camera
      index=0;
      for (port=0;port<portmax;port++) {
	if (handles[port]!=0)
	  for (cam=0;cam<port_camera_num[port];cam++) {
	    camera_ptr=NewCamera();
	    GetCameraData(handles[port], camera_nodes[port][cam], camera_ptr);
	    AppendCamera(camera_ptr);
	  }
      }

      raw1394_set_bus_reset_handler(handles[0], bus_reset_handler);
      GrabSelfIds(handles, portmax);
      silent_ui_update=0;
      SetChannels();
      // current camera is the first camera:
      SetCurrentCamera(cameras->camera_info.euid_64);
      // Create the permanent control windows.
      // (note BTW that other windows like 'file_selector' are created
      //  and destroyed on purpose while the following windows always exist.)
      
      g_thread_init(NULL);
      
      preferences_window= create_preferences_window();
      main_window = create_main_window();
      absolute_settings_window = create_absolute_settings_window();
      
      // Setup the GUI in accordance with the camera capabilities
      GetContextStatus();
      BuildAllWindows();
      UpdateAllWindows();
#ifdef HAVE_SDLLIB
      WatchStartThread(&watchthread_info);
#endif
      MainStatus("Welcome to Coriander...");
      gtk_widget_show (main_window); // this is the only window shown at boot-time
      
      main_timeout=gtk_timeout_add(10, (GtkFunction)main_timeout_handler, (gpointer*)portmax);
      //gdk_threads_enter();
      gtk_main();
      //gdk_threads_leave();
      
      // clean all threads for all cams:
      camera=cameras;
      while (camera!=NULL) {
	FtpStopThread();
	SaveStopThread();
	DisplayStopThread();
	IsoStopThread();
	camera=camera->next;
      }
      
#ifdef HAVE_SDLLIB
      WatchStopThread(&watchthread_info);
#endif

      gtk_timeout_remove(main_timeout);

      while (cameras!=NULL) {
	RemoveCamera(cameras->camera_info.euid_64);
      }

      for (i=0;i<portmax;i++) {
	if (handles[i]!=0)
	  raw1394_destroy_handle(handles[i]);
	free(camera_nodes[i]);
      }
      
      free(camera_nodes);
      free(handles);
      free(port_camera_num);
      
    } // end of if no handle check
    
  } // end of if no camera check
  //free(whitebal_data);
  
  return 0;
}
