/*
 *                     C  O  R  I  A  N  D  E  R
 *
 *            -- The IEEE-1394 Digital Camera controller --
 *
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
 *               Video Overlay by Dan Dennedy <ddennedy@coolsite.net>
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
#include "preferences.h"
#include "thread_base.h"
#include "raw1394support.h"
#include <libdc1394/dc1394_control.h>
#include <libraw1394/raw1394.h>

GtkWidget *commander_window;
GtkWidget **porthole_windows;
GtkWidget *porthole_window;
GtkWidget *about_window;
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
//int porthole_is_open;
int silent_ui_update;

int camera_num;
int current_camera;

int
main (int argc, char *argv[])
{
  int err, i;
  nodeid_t *camera_nodes; 
  raw1394handle_t handle;
  char tmp[256];
  int port=0; // port 0 is the first IEEE1394 card. We ONLY probe this one.

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

  g_thread_init(NULL);
  gnome_init ("coriander", VERSION, argc, argv);
  gdk_imlib_init();

  handle=dc1394_create_handle(port);
  // probe the IEEE1394 bus for DC camera:
  if (handle==0)
    {
      gtk_widget_show(create_no_handle_window());
      gtk_main ();
    }
  else
    {
  camera_nodes=dc1394_get_camera_nodes(handle, &camera_num, 0); // 0 not to show the cams.
  if (camera_num<1)
    {
      gtk_widget_show(create_no_camera_window());
      gtk_main ();
    }
  else
    {
  
      //fprintf(stderr,"%d\n",camera_num);
  // allocate memory space for all camera infos & download all infos:
  cameras=(dc1394_camerainfo*)calloc(camera_num,sizeof(dc1394_camerainfo));
  feature_sets=(dc1394_feature_set*)calloc(camera_num,sizeof(dc1394_feature_set));
  misc_infos=(dc1394_miscinfo*)calloc(camera_num,sizeof(dc1394_miscinfo));
  image_pipes=(chain_t**)calloc(camera_num,sizeof(chain_t*));
  format7_infos=(Format7Info*)calloc(camera_num,sizeof(Format7Info));
  uiinfos=(UIInfo*)calloc(camera_num,sizeof(UIInfo));
  selfids=(SelfIdPacket_t*)calloc(camera_num,sizeof(SelfIdPacket_t));
  porthole_windows=(GtkWidget**)calloc(camera_num,sizeof(GtkWidget*));

  err=1;
  for (i=0;i<camera_num;i++)
    {
      err*=dc1394_get_camera_misc_info(handle, camera_nodes[i], &misc_infos[i]);
      err*=dc1394_get_camera_info(handle, camera_nodes[i], &cameras[i]);
      err*=dc1394_get_camera_feature_set(handle, cameras[i].id, &feature_sets[i]);
      if (!err) MainError("Could not get camera basic informations!");
      GetFormat7Capabilities(handle, cameras[i].id, &format7_infos[i]);
      image_pipes[i]=NULL;
      uiinfos[i].test_pattern=0;
      uiinfos[i].want_to_display=0;
      // multiple porthole windows:
      porthole_windows[i] = create_porthole_window();
      sprintf(tmp,"Node %d: %s",cameras[i].id,cameras[i].model);
      gtk_window_set_title(GTK_WINDOW(porthole_windows[i]),tmp);
      gtk_signal_connect (GTK_OBJECT (porthole_windows[i]), "destroy_event",
			  GTK_SIGNAL_FUNC (on_porthole_window_close), (int*)i);
      gtk_signal_connect (GTK_OBJECT (porthole_windows[i]), "destroy",
			  GTK_SIGNAL_FUNC (on_porthole_window_close), (int*)i);
      gtk_signal_connect (GTK_OBJECT (porthole_windows[i]), "delete_event",
			  GTK_SIGNAL_FUNC (on_porthole_window_close), (int*)i);
    }
  GrabSelfIds(handle);
  silent_ui_update=0;

  for (i=0;i<camera_num;i++)
    SetChannel(i);

  // current camera is the first camera:
  SelectCamera(0);

  // Create the permanent control windows.
  // (note BTW that other windows like 'file_selector' are created
  //  and destroyed on purpose while the following windows always exist.)
  preferences_window= create_preferences_window();
  commander_window = create_commander_window();
  format7_window = create_format7_window();

  // Setup the GUI in accordance with the camera capabilities
  GetContextStatus();
  BuildAllWindows();
  UpdateAllWindows();

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

  free(cameras);
  free(feature_sets);
  free(misc_infos);
  free(image_pipes);
  free(format7_infos);
  free(uiinfos);
  free(selfids);
  free(porthole_windows);

    } // end of if no handle check
  raw1394_destroy_handle(handle);

    } // end of if no camera check
  

  return 0;
}
