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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <gnome.h>
#include "definitions.h"
#include "tools.h"

typedef struct _PrefsInfo
{
  float op_timeout;
  int auto_update;
  float auto_update_frequency;
  int display_keep_ratio;
  int display_period;
  int receive_method;
  char *video1394_device;
  int video1394_dropframes;
  char *save_filename;
  int save_scratch;
  int save_period;
  int save_convert;
  char *ftp_address;
  char *ftp_user;
  char *ftp_password;
  char *ftp_filename;
  char *ftp_path;
  int ftp_scratch;
  int ftp_period;

  int v4l_period;
  char *v4l_dev_name;

  char **camera_names;

  // internal data:
  int receive_method2index[2];

} PrefsInfo;

void
LoadConfigFile(void);

#endif
