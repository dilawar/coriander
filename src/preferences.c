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

#include "preferences.h"

extern PrefsInfo preferences; 
extern camera_t* camera;
extern camera_t* cameras;
extern int camera_num;

void
LoadConfigFile(void)
{
  camera_t* camera_ptr;
  char tmp[1024];

  preferences.op_timeout = gnome_config_get_float("coriander/global/one_push_timeout=10.0");
  preferences.auto_update = gnome_config_get_int("coriander/global/auto_update=1");
  preferences.auto_update_frequency = gnome_config_get_float("coriander/global/auto_update_frequency=2.0");
  preferences.display_keep_ratio = gnome_config_get_int("coriander/display/keep_ratio=0");
  preferences.display_period = gnome_config_get_int("coriander/display/period=1");
  preferences.receive_method = gnome_config_get_int("coriander/receive/method=0");
  preferences.video1394_device = gnome_config_get_string("coriander/receive/video1394_device=/dev/video1394/0");
  preferences.video1394_dropframes = gnome_config_get_int("coriander/receive/video1394_dropframes=0");
  preferences.save_filename = gnome_config_get_string("coriander/save/filename=test.jpg");
  preferences.save_scratch = gnome_config_get_int("coriander/save/scratch=0");
  preferences.save_period = gnome_config_get_int("coriander/save/period=1");
  preferences.save_convert = gnome_config_get_int("coriander/save/convert=1");
  preferences.ftp_address = gnome_config_get_string("coriander/ftp/address=");
  preferences.ftp_user = gnome_config_get_string("coriander/ftp/user=username");
  preferences.ftp_password = gnome_config_get_string("coriander/ftp/password=");
  preferences.ftp_filename = gnome_config_get_string("coriander/ftp/filename=");
  preferences.ftp_path = gnome_config_get_string("coriander/ftp/path=");
  preferences.ftp_scratch = gnome_config_get_int("coriander/ftp/scratch=0");
  preferences.ftp_period = gnome_config_get_int("coriander/ftp/period=1");
  preferences.v4l_period = gnome_config_get_int("coriander/v4l/period=1");
  preferences.v4l_dev_name = gnome_config_get_string("coriander/v4l/v4l_dev_name=/dev/video0");

  camera_ptr=cameras;
  while (camera_ptr!=NULL) {
    sprintf(tmp,"coriander/camera_names/%llx=%s %s",camera_ptr->camera_info.euid_64, camera_ptr->camera_info.vendor, camera_ptr->camera_info.model);
    camera_ptr->name = gnome_config_get_string(tmp);
    camera_ptr=camera_ptr->next;
  }

}
