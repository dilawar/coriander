/*
 *                     C  O  R  I  A  N  D  E  R
 *
 *            -- The IEEE-1394 Digital Camera controller --
 *
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#include "coriander.h"

int
main (int argc, char *argv[])
{
  int err;
  int main_timeout;
  GtkWidget* err_window;
  raw1394handle_t tmp_handle;

  main_timeout_ticker=0;
  WM_cancel_display=0;
  cameras=NULL;
  silent_ui_update=0;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif
  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

  GetXvInfo(&xvinfo);
  //fprintf(stderr,"%d %d\n", xvinfo.max_height, xvinfo.max_width);
  LoadConfigFile();

  //  port_num should be set here or later below
  tmp_handle=raw1394_new_handle();
  if (tmp_handle==NULL) {
    err_window=create_no_handle_window();
    gtk_widget_show(err_window);
    gtk_main();
    return(1);
  }
  port_num=raw1394_get_port_info(tmp_handle, NULL, 0);
  raw1394_destroy_handle(tmp_handle);

  err=GetCameraNodes();

  if (err==DC1394_NO_CAMERA) {
    err_window=create_no_camera_window();
    gtk_widget_show(err_window);
    gtk_main();
    return(1);
  }
  else if (err!=DC1394_SUCCESS) {
    fprintf(stderr, "Unknown error getting cameras on the bus.\nExiting\n");
    exit(1);
  }

  GrabSelfIds(cameras);

  SetChannels();
  // current camera is the first camera:
  SetCurrentCamera(cameras->camera_info.euid_64);
  
  preferences_window= create_preferences_window();
  main_window = create_main_window();
  
  format7_tab_presence=1;
  gtk_notebook_set_homogeneous_tabs(GTK_NOTEBOOK(lookup_widget(main_window,"notebook2")),TRUE);
  gtk_notebook_set_homogeneous_tabs(GTK_NOTEBOOK(lookup_widget(main_window,"notebook5")),TRUE);

  // Setup the GUI in accordance with the camera capabilities
  GetContextStatus();
  BuildAllWindows();
  UpdateAllWindows();

  MainStatus("Welcome to Coriander...");
  gtk_widget_show (main_window); // this is the only window shown at boot-time
  
  main_timeout=gtk_timeout_add(10, (GtkFunction)main_timeout_handler, (gpointer*)port_num);

#ifdef HAVE_SDLLIB
  WatchStartThread(&watchthread_info);
#endif

  gtk_main();

  gtk_timeout_remove(main_timeout);
  
#ifdef HAVE_SDLLIB
  WatchStopThread(&watchthread_info);
#endif

  while (cameras!=NULL) {
    RemoveCamera(cameras->camera_info.euid_64);
  }
  /*
  for (i=0;i<businfo->port_num;i++) {
    if (businfo->handles[i]!=0)
      raw1394_destroy_handle(businfo->handles[i]);
    free(businfo->camera_nodes[i]);
  }
  
  free(businfo->camera_nodes);
  free(businfo->handles);
  free(businfo->port_camera_num);
  */
  return 0;
}
