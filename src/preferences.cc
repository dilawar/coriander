/*
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
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
#include <libdc1394/dc1394_control.h>
#include "thread_real.h"

extern "C" {

#include "callbacks.h"
#include "support.h" 
#include "definitions.h"
#include "preferences.h"
#include "thread_display.h"
#include "thread_iso.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "tools.h"

extern PrefsInfo preferences; 
extern dc1394_camerainfo *cameras;
extern dc1394_miscinfo *misc_info;
extern dc1394_feature_set *feature_set;
extern int camera_num;

void
LoadConfigFile(void)
{
  int i;

  char tmp[1024];

  preferences.op_timeout = gnome_config_get_float("coriander/global/one_push_timeout=10.0");
  preferences.auto_update = gnome_config_get_int("coriander/global/auto_update=1");
  preferences.auto_update_frequency = gnome_config_get_float("coriander/global/auto_update_frequency=2.0");
  preferences.display_keep_ratio = gnome_config_get_int("coriander/display/keep_ratio=0");
  preferences.display_period = gnome_config_get_int("coriander/display/period=1");
  preferences.receive_method = gnome_config_get_int("coriander/receive/method=0");
  preferences.video1394_device = gnome_config_get_string("coriander/receive/video1394_device=/dev/video1394");
  preferences.video1394_dropframes = gnome_config_get_int("coriander/receive/video1394_dropframes=0");
  preferences.save_filename = gnome_config_get_string("coriander/save/filename=test.jpg");
  preferences.save_scratch = gnome_config_get_int("coriander/save/scratch=0");
  preferences.save_period = gnome_config_get_int("coriander/save/period=1");
  preferences.save_convert = gnome_config_get_int("coriander/save/convert=1");
  preferences.ftp_address = gnome_config_get_string("coriander/ftp/address=ftp.sf.net");
  preferences.ftp_user = gnome_config_get_string("coriander/ftp/user=username");
  preferences.ftp_password = gnome_config_get_string("coriander/ftp/password=don'tyouwish");
  preferences.ftp_filename = gnome_config_get_string("coriander/ftp/filename=helloworld.jpg");
  preferences.ftp_path = gnome_config_get_string("coriander/ftp/path=/pub/");
  preferences.ftp_scratch = gnome_config_get_int("coriander/ftp/scratch=0");
  preferences.ftp_period = gnome_config_get_int("coriander/ftp/period=1");
  preferences.real_address = gnome_config_get_string("coriander/real/address=your.server.address");
  preferences.real_user = gnome_config_get_string("coriander/real/user=username");
  preferences.real_password = gnome_config_get_string("coriander/real/password=don'tyouwish");
  preferences.real_filename = gnome_config_get_string("coriander/real/filename=helloworld.rm");
  preferences.real_port = gnome_config_get_int("coriander/real/port=4040");
  preferences.real_title = gnome_config_get_string("coriander/real/title=my stream");
  preferences.real_author = gnome_config_get_string("coriander/real/author=Myself");
  preferences.real_copyright = gnome_config_get_string("coriander/real/copyright=(c)2002");
  preferences.real_recordable = gnome_config_get_int("coriander/real/recordable=1");
  preferences.real_audience = gnome_config_get_int("coriander/real/audience=0");
  preferences.real_quality = gnome_config_get_int("coriander/real/quality=0");
  preferences.real_compatibility = gnome_config_get_int("coriander/real/compatibility=0");
  preferences.real_period = gnome_config_get_int("coriander/real/period=1");

  for (i=0;i<camera_num;i++)
    {
      sprintf(tmp,"coriander/camera_names/%llx=%s %s",cameras[i].euid_64, cameras[i].vendor, cameras[i].model);
      preferences.camera_names[i] = gnome_config_get_string(tmp);
    }
}


void
SaveSetup(char *filename)
{
  FILE *fd;
  int i;
  fd=fopen(filename,"w");
  if (fd==NULL)
    MainError("Can't open file for saving");
  else
    {
      fprintf(fd,"%d\n",misc_info->format);
      fprintf(fd,"%d\n",misc_info->mode);
      fprintf(fd,"%d\n",misc_info->framerate);
      fprintf(fd,"%d\n",misc_info->is_iso_on);
      fprintf(fd,"%d\n",misc_info->iso_channel);
      fprintf(fd,"%d\n",misc_info->mem_channel_number);
      fprintf(fd,"%d\n",misc_info->save_channel);
      fprintf(fd,"%d\n",misc_info->load_channel);
      for (i=0;i<NUM_FEATURES;i++)
	{
	  switch (i+FEATURE_MIN)
	    { 
	    case FEATURE_TRIGGER:
	      fprintf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fprintf(fd,"%d\n",feature_set->feature[i].trigger_mode);
	      fprintf(fd,"%d\n",feature_set->feature[i].trigger_polarity);
	      fprintf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    case FEATURE_WHITE_BALANCE:
	      fprintf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fprintf(fd,"%d\n",feature_set->feature[i].BU_value);
	      fprintf(fd,"%d\n",feature_set->feature[i].RV_value);
	      break;
	    case FEATURE_TEMPERATURE:
	      fprintf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fprintf(fd,"%d\n",feature_set->feature[i].target_value);
	      fprintf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    default:
	      fprintf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fprintf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    }
	}
      fclose(fd);
    }
}

void
LoadSetup(char *filename)
{
  FILE *fd;
  int i;

  fd=fopen(filename,"r");
  if (fd==NULL)
    MainError("Can't open file for loading");
  else
    {
      fscanf(fd,"%d\n",misc_info->format);
      fscanf(fd,"%d\n",misc_info->mode);
      fscanf(fd,"%d\n",misc_info->framerate);
      fscanf(fd,"%d\n",misc_info->is_iso_on);
      fscanf(fd,"%d\n",misc_info->iso_channel);
      fscanf(fd,"%d\n",misc_info->mem_channel_number);
      fscanf(fd,"%d\n",misc_info->save_channel);
      fscanf(fd,"%d\n",misc_info->load_channel);
      for (i=0;i<NUM_FEATURES;i++)
	{
	  switch (i+FEATURE_MIN)
	    { 
	    case FEATURE_TRIGGER:
	      fscanf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fscanf(fd,"%d\n",feature_set->feature[i].trigger_mode);
	      fscanf(fd,"%d\n",feature_set->feature[i].trigger_polarity);
	      fscanf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    case FEATURE_WHITE_BALANCE:
	      fscanf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fscanf(fd,"%d\n",feature_set->feature[i].BU_value);
	      fscanf(fd,"%d\n",feature_set->feature[i].RV_value);
	      break;
	    case FEATURE_TEMPERATURE:
	      fscanf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fscanf(fd,"%d\n",feature_set->feature[i].target_value);
	      fscanf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    default:
	      fscanf(fd,"%d\n",feature_set->feature[i].auto_active);
	      fscanf(fd,"%d\n",feature_set->feature[i].value);
	      break;
	    }
	}
      // REDRAW ALL HERE...

      ///////
      fclose(fd);
    }
}

}

