/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
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

#define PREFERENCE_ITEMS 29

enum {
  ONE_PUSH_TIMEOUT=0,
  AUTO_UPDATE,
  AUTO_UPDATE_FREQUENCY,
  DISPLAY_METHOD,
  DISPLAY_PERIOD,
  RECEIVE_METHOD,
  SAVE_FILENAME, 
  SAVE_SCRATCH,
  SAVE_PERIOD,
  FTP_ADDRESS,
  FTP_USER,
  FTP_PASSWORD,
  FTP_FILENAME,
  FTP_PATH,
  FTP_SCRATCH,
  FTP_PERIOD,
  REAL_ADDRESS,
  REAL_USER,
  REAL_PASSWORD,
  REAL_FILENAME,
  REAL_PORT,
  REAL_TITLE,
  REAL_AUTHOR,
  REAL_COPYRIGHT,
  REAL_RECORDABLE,
  REAL_AUDIENCE,
  REAL_QUALITY,
  REAL_COMPATIBILITY,
  REAL_PERIOD
};

typedef struct _PrefsInfo
{
  float op_timeout;
  int auto_update;
  float auto_update_frequency;
  int display_method;
  int display_period;
  int receive_method;
  char save_filename[256];
  int save_scratch;
  int save_period;
  char ftp_address[256];
  char ftp_user[256];
  char ftp_password[256];
  char ftp_filename[256];
  char ftp_path[256];
  int ftp_scratch;
  int ftp_period;
  char real_address[256];
  char real_user[256];
  char real_password[256];
  char real_filename[256];
  int real_port;
  char real_author[256];
  char real_title[256];
  char real_copyright[256];
  int real_recordable;
  int real_audience;
  int real_quality;
  int real_compatibility;
  int real_period;

} PrefsInfo;

#ifdef __cplusplus
extern "C" {
#endif

void
ParseConfigFile(FILE* fd);

void
SetPreferencesDefaults(void);

void
LoadConfigFile(void);

void
WriteConfigFile(void);

char *
GetFileName(void);

#ifdef __cplusplus
}
#endif

#endif
