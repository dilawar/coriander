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

extern char * preferences_features[PREFERENCE_ITEMS];
extern char * preferences_defaults[PREFERENCE_ITEMS];
extern PrefsInfo preferences; 

void
SetPreferencesDefaults(void)
{
  preferences.op_timeout=10;
  preferences.auto_update=0;
  preferences.auto_update_frequency=10;
  preferences.display_keep_ratio=0;;
  preferences.display_period=1;
  preferences.receive_method=RECEIVE_METHOD_RAW1394;
  strcpy(preferences.save_filename,"test.jpg");
  preferences.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
  preferences.save_period=1;
  strcpy(preferences.ftp_address,"ftp.sf.net");
  strcpy(preferences.ftp_user,"user_string");
  strcpy(preferences.ftp_password,"password_string");
  strcpy(preferences.ftp_filename,"hello_world.bmp");
  strcpy(preferences.ftp_path,"/path/");
  preferences.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
  preferences.ftp_period=1;
  strcpy(preferences.real_address,"your.server.address");
  strcpy(preferences.real_user,"user_string");
  strcpy(preferences.real_password,"password_string");
  strcpy(preferences.real_filename,"hello_world.rm");
  preferences.real_port=4040;
  strcpy(preferences.real_title,"Hello World");
  strcpy(preferences.real_author,"Myself");
  strcpy(preferences.real_copyright,"(c)2032 Jenkins the Robot");
  preferences.real_recordable=0;
#ifdef HAVE_REALLIB
  preferences.real_audience=REAL_AUDIENCE_LAN_HIGH;
  preferences.real_quality=REAL_QUALITY_NORMAL;
  preferences.real_compatibility=REAL_COMPATIBILITY_6;
#else
  preferences.real_audience=0;
  preferences.real_quality=0;
  preferences.real_compatibility=0;
  preferences.real_period=1;
#endif
}

void
LoadConfigFile(void)
{
  FILE* fd;
  char *filename;

  SetPreferencesDefaults();

  filename=GetFileName();
  fd=fopen(filename,"r");
  free(filename);

  if (fd==NULL)
    {
      MainStatus("No config file yet: we create a default one");
      // file not available, we create a default one
      WriteConfigFile(); // defaults are used.
    }
  else
    {
      ParseConfigFile(fd);
      fclose(fd);
    }
}


void
WriteConfigFile(void)
{
  FILE* fd;
  char *filename;
  
  filename=GetFileName();
  fd=fopen(filename,"w");
  free(filename);

  fprintf(fd,"%s %f\n",preferences_features[0],preferences.op_timeout);
  fprintf(fd,"%s %f\n",preferences_features[1],preferences.auto_update);
  fprintf(fd,"%s %f\n",preferences_features[2],preferences.auto_update_frequency);
  fprintf(fd,"%s %d\n",preferences_features[3],preferences.display_keep_ratio);
  fprintf(fd,"%s %d\n",preferences_features[4],preferences.display_period);
  fprintf(fd,"%s %d\n",preferences_features[5],preferences.receive_method);
  fprintf(fd,"%s %s\n",preferences_features[6],preferences.save_filename);
  fprintf(fd,"%s %d\n",preferences_features[7],preferences.save_scratch);
  fprintf(fd,"%s %d\n",preferences_features[8],preferences.save_period);
  fprintf(fd,"%s %s\n",preferences_features[9],preferences.ftp_address);
  fprintf(fd,"%s %s\n",preferences_features[10],preferences.ftp_user);
  fprintf(fd,"%s %s\n",preferences_features[11],preferences.ftp_password);
  fprintf(fd,"%s %s\n",preferences_features[12],preferences.ftp_filename);
  fprintf(fd,"%s %s\n",preferences_features[13],preferences.ftp_path);
  fprintf(fd,"%s %d\n",preferences_features[14],preferences.ftp_scratch);
  fprintf(fd,"%s %d\n",preferences_features[15],preferences.ftp_period);
  fprintf(fd,"%s %s\n",preferences_features[16],preferences.real_address);
  fprintf(fd,"%s %s\n",preferences_features[17],preferences.real_user);
  fprintf(fd,"%s %s\n",preferences_features[18],preferences.real_password);
  fprintf(fd,"%s %s\n",preferences_features[19],preferences.real_filename);
  fprintf(fd,"%s %d\n",preferences_features[20],preferences.real_port);
  fprintf(fd,"%s %s\n",preferences_features[21],preferences.real_author);
  fprintf(fd,"%s %s\n",preferences_features[22],preferences.real_title);
  fprintf(fd,"%s %s\n",preferences_features[23],preferences.real_copyright);
  fprintf(fd,"%s %s\n",preferences_features[24],preferences.real_recordable);
  fprintf(fd,"%s %ld\n",preferences_features[25],preferences.real_audience);
  fprintf(fd,"%s %d\n",preferences_features[26],preferences.real_quality);
  fprintf(fd,"%s %d\n",preferences_features[27],preferences.real_compatibility);

  fclose(fd);
}


void
ParseConfigFile(FILE* fd)
{

  char feature_string[STRING_SIZE],feature_value[STRING_SIZE];
  int check,feature_id, i;
  char *needle;
  check=fscanf(fd,"%s %s",feature_string, feature_value);

  while(check!=EOF)
    {
      feature_id=-1;
      // find the current feature for feature_string
      for (i=0;i<PREFERENCE_ITEMS;i++)
	{
	  needle=strstr(feature_string,preferences_features[i]);
	  if (needle!=NULL)
	    feature_id=i;
	}
      if (feature_id<0)
	MainError("Invalid config item");
      else
	{
	  switch(feature_id)
	    {
	    case ONE_PUSH_TIMEOUT:
	      preferences.op_timeout=atof(feature_value);
	      break;
	    case AUTO_UPDATE:
	      preferences.auto_update=atoi(feature_value);
	      break;
	    case AUTO_UPDATE_FREQUENCY:
	      preferences.auto_update_frequency=atof(feature_value);
	      break;
	    case DISPLAY_KEEP_RATIO:
	      preferences.display_keep_ratio=atoi(feature_value);
	      break;
	    case DISPLAY_PERIOD:
	      preferences.display_period=atoi(feature_value);
	      break;
	    case RECEIVE_METHOD:
	      preferences.receive_method=atoi(feature_value);
	      break;
	    case SAVE_FILENAME:
	      strcpy(preferences.save_filename,feature_value);
	      break;
	    case SAVE_SCRATCH:
	      preferences.save_scratch=atoi(feature_value);
	      break;
	    case SAVE_PERIOD:
	      preferences.save_period=atoi(feature_value);
	      break;
	    case FTP_ADDRESS:
	      strcpy(preferences.ftp_address,feature_value);
	      break;
	    case FTP_USER:
	      strcpy(preferences.ftp_user,feature_value);
	      break;
	    case FTP_PASSWORD:
	      strcpy(preferences.ftp_password,feature_value);
	      break;
	    case FTP_FILENAME:
	      strcpy(preferences.ftp_filename,feature_value);
	      break;
	    case FTP_PATH:
	      strcpy(preferences.ftp_path,feature_value);
	      break;
	    case FTP_SCRATCH:
	      preferences.ftp_scratch=atoi(feature_value);
	      break;
	    case FTP_PERIOD:
	      preferences.ftp_period=atoi(feature_value);
	      break;
	    case REAL_ADDRESS:
	      strcpy(preferences.real_address,feature_value);
	      break;
	    case REAL_USER:
	      strcpy(preferences.real_user,feature_value);
	      break;
	    case REAL_PASSWORD:
	      strcpy(preferences.real_password,feature_value);
	      break;
	    case REAL_FILENAME:
	      strcpy(preferences.real_filename,feature_value);
	      break;
	    case REAL_PORT:
	      preferences.real_port=atoi(feature_value);
	      break;
	    case REAL_TITLE:
	      strcpy(preferences.real_title,feature_value);
	      break;
	    case REAL_AUTHOR:
	      strcpy(preferences.real_author,feature_value);
	      break;
	    case REAL_COPYRIGHT:
	      strcpy(preferences.real_copyright,feature_value);
	      break;
	    case REAL_RECORDABLE:
	      preferences.real_recordable=atoi(feature_value);
	      break;
	    case REAL_AUDIENCE:
	      preferences.real_audience=atoi(feature_value);
	      break;
	    case REAL_QUALITY:
	      preferences.real_quality=atoi(feature_value);
	      break;
	    case REAL_COMPATIBILITY:
	      preferences.real_compatibility=atoi(feature_value);
	      break;
	    case REAL_PERIOD:
	      preferences.real_period=atoi(feature_value);
	    default:
	      MainError("Invalid config item id");
	      break;
	    }
	}
      check=fscanf(fd,"%s %s",feature_string, feature_value);
    }
 
}

char *
GetFileName(void)
{
  char *path;
  char *out;
  const char * filename="/.coriander";

  out=(char*)malloc(STRING_SIZE*sizeof(char));

  path=getenv("HOME");

  if (path==NULL)
    {
      MainError("Can't get HOME environment variable!");
    }
  else
    {
      sprintf(out,"%s%s",path,filename);
    }
  return(out);
}

}
