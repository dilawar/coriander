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

extern Prefs_t preferences; 
extern camera_t* camera;
extern camera_t* cameras;
extern int camera_num;

void
LoadConfigFile(void)
{
  
  preferences.camprefs.display_keep_ratio   = gnome_config_get_int("coriander/display/keep_ratio=0");
  preferences.camprefs.display_period       = gnome_config_get_int("coriander/display/period=1");
  preferences.camprefs.display_redraw       = gnome_config_get_int("coriander/display/redraw=1");
  preferences.camprefs.display_redraw_rate  = gnome_config_get_float("coriander/display/redraw_rate=4.0");
  preferences.camprefs.receive_method       = gnome_config_get_int("coriander/receive/method=0");
  preferences.camprefs.dma_buffer_size      = gnome_config_get_int("coriander/receive/dma_buffer_size=10");
  preferences.camprefs.video1394_dropframes = gnome_config_get_int("coriander/receive/video1394_dropframes=0");
  preferences.camprefs.save_scratch         = gnome_config_get_int("coriander/save/scratch=0");
  preferences.camprefs.save_period          = gnome_config_get_int("coriander/save/period=1");
  preferences.camprefs.save_convert         = gnome_config_get_int("coriander/save/convert=0");
  preferences.camprefs.save_datenum         = gnome_config_get_int("coriander/save/datenum=1");
  preferences.camprefs.use_ram_buffer       = gnome_config_get_int("coriander/save/use_ram_buffer=0");
  preferences.camprefs.ram_buffer_size      = gnome_config_get_int("coriander/save/ram_buffer_size=100");
  preferences.camprefs.ftp_scratch          = gnome_config_get_int("coriander/ftp/scratch=0");
  preferences.camprefs.ftp_period           = gnome_config_get_int("coriander/ftp/period=1");
  preferences.camprefs.ftp_datenum          = gnome_config_get_int("coriander/ftp/datenum=1");
  preferences.camprefs.v4l_period           = gnome_config_get_int("coriander/v4l/period=1");

  //fprintf(stderr,"ftpuser orig adr: 0x%x\n",preferences.camprefs.video1394_device);
  //preferences.camprefs.video1394_device=(char*)malloc(STRING_SIZE*sizeof(char));
  //fprintf(stderr,"ftpuser orig adr: 0x%x\n",preferences.camprefs.video1394_device);
  //sprintf(preferences.camprefs.video1394_device,"/dev/video1394/%d", dc1394_get_camera_port(camera->camera_info.handle));
  preferences.camprefs.video1394_device = gnome_config_get_string("coriander/receive/video1394_device=/dev/video1394/0");

  preferences.camprefs.save_filename = gnome_config_get_string("coriander/save/filename=test.jpg");
  preferences.camprefs.ftp_filename  = gnome_config_get_string("coriander/ftp/filename=");
  preferences.camprefs.ftp_path      = gnome_config_get_string("coriander/ftp/path=");
  preferences.camprefs.ftp_address   = gnome_config_get_string("coriander/ftp/address=");
  preferences.camprefs.ftp_user      = gnome_config_get_string("coriander/ftp/user=username");
  preferences.camprefs.v4l_dev_name  = gnome_config_get_string("coriander/v4l/v4l_dev_name=/dev/video0");
  preferences.camprefs.ftp_password = "";

  preferences.op_timeout             = gnome_config_get_float("coriander/global/one_push_timeout=10.0");
  preferences.auto_update            = gnome_config_get_int("coriander/global/auto_update=1");
  preferences.auto_update_frequency  = gnome_config_get_float("coriander/global/auto_update_frequency=2.0");
  preferences.sync_control           = gnome_config_get_float("coriander/global/sync_control=0");
}

void
CopyCameraPrefs(camera_t* cam) {

  char *tmp, *tmp_ptr;

  cam->prefs.display_keep_ratio     = preferences.camprefs.display_keep_ratio;
  cam->prefs.display_period         = preferences.camprefs.display_period;
  cam->prefs.display_redraw         = preferences.camprefs.display_redraw;
  cam->prefs.display_redraw_rate    = preferences.camprefs.display_redraw_rate;
  cam->prefs.receive_method         = preferences.camprefs.receive_method;
  cam->prefs.video1394_dropframes   = preferences.camprefs.video1394_dropframes;
  cam->prefs.dma_buffer_size        = preferences.camprefs.dma_buffer_size;
  cam->prefs.save_scratch           = preferences.camprefs.save_scratch;
  cam->prefs.save_period            = preferences.camprefs.save_period;
  cam->prefs.save_datenum           = preferences.camprefs.save_datenum;
  cam->prefs.ram_buffer_size        = preferences.camprefs.ram_buffer_size;
  cam->prefs.ftp_scratch            = preferences.camprefs.ftp_scratch;
  cam->prefs.ftp_period             = preferences.camprefs.ftp_period;
  cam->prefs.ftp_datenum            = preferences.camprefs.ftp_datenum;
  cam->prefs.v4l_period             = preferences.camprefs.v4l_period;
  strcpy(cam->prefs.save_filename   , preferences.camprefs.save_filename);
  strcpy(cam->prefs.ftp_filename    , preferences.camprefs.ftp_filename);
  strcpy(cam->prefs.ftp_path        , preferences.camprefs.ftp_path);
  strcpy(cam->prefs.ftp_address     , preferences.camprefs.ftp_address);
  strcpy(cam->prefs.ftp_user        , preferences.camprefs.ftp_user);
  strcpy(cam->prefs.v4l_dev_name    , preferences.camprefs.v4l_dev_name);
  strcpy(cam->prefs.video1394_device, preferences.camprefs.video1394_device);
  preferences.camprefs.ftp_password = "";
  
  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(tmp,"coriander/camera_names/%llx=%s %s",cam->camera_info.euid_64,
	  cam->camera_info.vendor, cam->camera_info.model);
  tmp_ptr=gnome_config_get_string(tmp);
  strcpy(cam->prefs.name,tmp_ptr);
  free(tmp);

}
