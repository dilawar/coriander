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

void
LoadConfigFile(void)
{
  preferences.op_timeout = gnome_config_get_int("coriander/global/one_push_timeout=10");
  preferences.auto_update = gnome_config_get_int("coriander/global/auto_update=0");
  preferences.auto_update_frequency = gnome_config_get_float("coriander/global/auto_update_frequency=10");
  preferences.display_keep_ratio = gnome_config_get_int("coriander/display/keep_ratio=0");
  preferences.display_period = gnome_config_get_int("coriander/display/period=1");
  preferences.receive_method = gnome_config_get_int("coriander/receive/method=0");
  g_free(preferences.video1394_device);
  preferences.video1394_device = gnome_config_get_string("coriander/receive/video1394_device=/dev/video1394");
  g_free(preferences.save_filename);
  preferences.save_filename = gnome_config_get_string("coriander/save/filename=test.jpg");
  preferences.save_scratch = gnome_config_get_int("coriander/save/scratch=0");
  preferences.save_period = gnome_config_get_int("coriander/save/period=1");
  g_free(preferences.ftp_address);
  preferences.ftp_address = gnome_config_get_string("coriander/ftp/address=ftp.sf.net");
  g_free(preferences.ftp_user);
  preferences.ftp_user = gnome_config_get_string("coriander/ftp/user=username");
  g_free(preferences.ftp_password);
  preferences.ftp_password = gnome_config_get_string("coriander/ftp/password=don'tyouwish");
  g_free(preferences.ftp_filename);
  preferences.ftp_filename = gnome_config_get_string("coriander/ftp/filename=helloworld.jpg");
  g_free(preferences.ftp_path);
  preferences.ftp_path = gnome_config_get_string("coriander/ftp/path=/pub/");
  preferences.ftp_scratch = gnome_config_get_int("coriander/ftp/scratch=0");
  preferences.ftp_period = gnome_config_get_int("coriander/ftp/period=1");
  g_free(preferences.real_address);
  preferences.real_address = gnome_config_get_string("coriander/real/address=your.server.address");
  g_free(preferences.real_user);
  preferences.real_user = gnome_config_get_string("coriander/real/user=username");
  g_free(preferences.real_password);
  preferences.real_password = gnome_config_get_string("coriander/real/password=don'tyouwish");
  g_free(preferences.real_filename);
  preferences.real_filename = gnome_config_get_string("coriander/real/filenamehelloworld.rm");
  preferences.real_port = gnome_config_get_int("coriander/real/port=4040");
  g_free(preferences.real_title);
  preferences.real_title = gnome_config_get_string("coriander/real/title=my stream");
  g_free(preferences.real_author);
  preferences.real_author = gnome_config_get_string("coriander/real/author=Myself");
  g_free(preferences.real_copyright);
  preferences.real_copyright = gnome_config_get_string("coriander/real/copyright=(c)2002");
  preferences.real_recordable = gnome_config_get_int("coriander/real/recordable=1");
  preferences.real_audience = gnome_config_get_int("coriander/real/audience=0");
  preferences.real_quality = gnome_config_get_int("coriander/real/quality=0");
  preferences.real_compatibility = gnome_config_get_int("coriander/real/compatibility=0");
  preferences.real_period = gnome_config_get_int("coriander/real/period=1");
}

void
WriteConfigFile(void)
{
  gnome_config_set_float("coriander/global/one_push_timeout",preferences.op_timeout);
  gnome_config_set_float("coriander/global/auto_update",preferences.auto_update);
  gnome_config_set_float("coriander/global/auto_update_frequency",preferences.auto_update_frequency);
  gnome_config_set_int("coriander/display/keep_ratio",preferences.display_keep_ratio);
  gnome_config_set_int("coriander/display/period",preferences.display_period);
  gnome_config_set_int("coriander/receive/method",preferences.receive_method);
  gnome_config_set_string("coriander/receive/video1394_device",preferences.video1394_device);
  gnome_config_set_string("coriander/save/filename",preferences.save_filename);
  gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
  gnome_config_set_int("coriander/save/period",preferences.save_period);
  gnome_config_set_string("coriander/ftp/address",preferences.ftp_address);
  gnome_config_set_string("coriander/ftp/user",preferences.ftp_user);
  gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  gnome_config_set_string("coriander/ftp/filename",preferences.ftp_filename);
  gnome_config_set_string("coriander/ftp/path",preferences.ftp_path);
  gnome_config_set_int("coriander/ftp/scratch",preferences.ftp_scratch);
  gnome_config_set_int("coriander/ftp/period",preferences.ftp_period);
  gnome_config_set_string("coriander/real/address",preferences.real_address);
  gnome_config_set_string("coriander/real/user",preferences.real_user);
  gnome_config_set_string("coriander/real/password",preferences.real_password);
  gnome_config_set_string("coriander/real/filename",preferences.real_filename);
  gnome_config_set_int("coriander/real/port",preferences.real_port);
  gnome_config_set_string("coriander/real/title",preferences.real_title);
  gnome_config_set_string("coriander/real/author",preferences.real_author);
  gnome_config_set_string("coriander/real/copyright",preferences.real_copyright);
  gnome_config_set_int("coriander/real/recordable",preferences.real_recordable);
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_set_int("coriander/real/quality",preferences.real_quality);
  gnome_config_set_int("coriander/real/compatibility",preferences.real_compatibility);
  gnome_config_set_int("coriander/real/period",preferences.real_period);

  gnome_config_sync();// ???
}


}
