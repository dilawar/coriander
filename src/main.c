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
GtkWidget *about_window;
GtkWidget *help_window;
GtkWidget *preferences_window;
GtkWidget *waiting_camera_window;
CtxtInfo ctxt;
PrefsInfo preferences;
int silent_ui_update;
camera_t* camera;
camera_t* cameras;

BusInfo_t* businfo;
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
  int i;
  int main_timeout;
  
  main_timeout_ticker=0;
  WM_cancel_display=0;
  cameras=NULL;

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  gnome_init ("coriander", VERSION, argc, argv);

  //whitebal_data=(whitebal_data_t*)malloc(sizeof(whitebal_data));

  businfo=(BusInfo_t*)malloc(sizeof(businfo));

  GetCameraNodes(businfo);

  if (businfo->card_found==0) {
    gtk_widget_show(create_no_handle_window());
    gtk_main ();
    free(businfo->camera_nodes);
    free(businfo->port_camera_num);
    free(businfo->handles);
    exit(1);
  }
  else {
    if (businfo->camera_num<1) {
      gtk_widget_show(create_no_camera_window());
      gtk_main ();
      free(businfo->camera_nodes);
      free(businfo->port_camera_num);
      free(businfo->handles);
      exit(1);
    }
  }

  // we have at least one camera on one interface card.
  // get camera infos and serialize the port info for each camera
  GetCamerasInfo(businfo);

  raw1394_set_bus_reset_handler(businfo->handles[0], bus_reset_handler);
  GrabSelfIds(businfo->handles, businfo->port_num);
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

  //absolute_settings_window = create_absolute_settings_window();
  
  // Setup the GUI in accordance with the camera capabilities
  GetContextStatus();
  BuildAllWindows();
  UpdateAllWindows();
#ifdef HAVE_SDLLIB
  WatchStartThread(&watchthread_info);
#endif
  MainStatus("Welcome to Coriander...");
  gtk_widget_show (main_window); // this is the only window shown at boot-time
  
  main_timeout=gtk_timeout_add(10, (GtkFunction)main_timeout_handler, (gpointer*)businfo->port_num);
  //gdk_threads_enter();
  gtk_main();
  //gdk_threads_leave();
#ifdef HAVE_SDLLIB
  WatchStopThread(&watchthread_info);
#endif
  
  gtk_timeout_remove(main_timeout);
  
  while (cameras!=NULL) {
    RemoveCamera(cameras->camera_info.euid_64);
  }
  
  for (i=0;i<businfo->port_num;i++) {
    if (businfo->handles[i]!=0)
      raw1394_destroy_handle(businfo->handles[i]);
    free(businfo->camera_nodes[i]);
  }
  
  free(businfo->camera_nodes);
  free(businfo->handles);
  free(businfo->port_camera_num);
  //free(whitebal_data);
  
  return 0;
}
