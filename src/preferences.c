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

#include "preferences.h"

extern PrefsUI_t preferences; 
extern camera_t* camera;
extern camera_t* cameras;
extern int camera_num;

void
LoadConfigFile(void)
{
  //camera_t* camera_ptr;

  preferences.op_timeout = gnome_config_get_float("coriander/global/one_push_timeout=10.0");
  preferences.auto_update = gnome_config_get_int("coriander/global/auto_update=1");
  preferences.auto_update_frequency = gnome_config_get_float("coriander/global/auto_update_frequency=2.0");
  /*
  camera_ptr=cameras;
  while (camera_ptr!=NULL) {

    LoadCameraConfig(camera_ptr);

    camera_ptr = camera_ptr->next;
  }
  */
}

void
LoadCameraConfig(camera_t* camera) {

  char *tmp;
    
  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  
  camera->prefs.display_keep_ratio = gnome_config_get_int("coriander/display/keep_ratio=0");
  camera->prefs.display_period = gnome_config_get_int("coriander/display/period=1");
  //camera->prefs.display_scale = gnome_config_get_int("coriander/display/scale=0");
  camera->prefs.display_redraw = gnome_config_get_int("coriander/display/redraw=1");
  camera->prefs.display_redraw_rate = gnome_config_get_float("coriander/display/redraw_rate=4.0");
  camera->prefs.receive_method = gnome_config_get_int("coriander/receive/method=0");
  camera->prefs.dma_buffer_size = gnome_config_get_int("coriander/receive/dma_buffer_size=10");
  //camera->prefs.video1394_device = gnome_config_get_string("coriander/receive/video1394_device=/dev/video1394/0");
  sprintf(camera->prefs.video1394_device,"/dev/video1394/%d", dc1394_get_camera_port(camera->camera_info.handle));
  camera->prefs.video1394_dropframes = gnome_config_get_int("coriander/receive/video1394_dropframes=0");
  strcpy(camera->prefs.save_filename, gnome_config_get_string("coriander/save/filename=test.jpg"));
  camera->prefs.save_scratch = gnome_config_get_int("coriander/save/scratch=0");
  camera->prefs.save_period = gnome_config_get_int("coriander/save/period=1");
  camera->prefs.save_convert = gnome_config_get_int("coriander/save/convert=0");
  camera->prefs.save_datenum = gnome_config_get_int("coriander/save/datenum=1");
  camera->prefs.use_ram_buffer = gnome_config_get_int("coriander/save/use_ram_buffer=0");
  camera->prefs.ram_buffer_size = gnome_config_get_int("coriander/save/ram_buffer_size=100");
  strcpy(camera->prefs.ftp_address,  gnome_config_get_string("coriander/ftp/address="));
  strcpy(camera->prefs.ftp_user,  gnome_config_get_string("coriander/ftp/user=username"));
  camera->prefs.ftp_password = "";
  strcpy(camera->prefs.ftp_filename,  gnome_config_get_string("coriander/ftp/filename="));
  strcpy(camera->prefs.ftp_path,  gnome_config_get_string("coriander/ftp/path="));
  camera->prefs.ftp_scratch = gnome_config_get_int("coriander/ftp/scratch=0");
  camera->prefs.ftp_period = gnome_config_get_int("coriander/ftp/period=1");
  camera->prefs.ftp_datenum = gnome_config_get_int("coriander/ftp/datenum=1");
  camera->prefs.v4l_period = gnome_config_get_int("coriander/v4l/period=1");
  strcpy(camera->prefs.v4l_dev_name,  gnome_config_get_string("coriander/v4l/v4l_dev_name=/dev/video0"));
  sprintf(tmp,"coriander/camera_names/%llx=%s %s",camera->camera_info.euid_64,
	  camera->camera_info.vendor, camera->camera_info.model);
  strcpy(camera->name, gnome_config_get_string(tmp));
  free(tmp);

}
