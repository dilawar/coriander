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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include "callbacks.h"
#include "support.h"
#include "definitions.h"
#include "preferences.h"
#include "thread_display.h"
#include "thread_iso.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern char * preferences_features[PREFERENCE_ITEMS];
extern char * preferences_defaults[PREFERENCE_ITEMS];
extern PrefsInfo preferences; 

void
SetPreferencesDefaults(void)
{
  preferences.op_timeout=10;
  preferences.auto_update=0;
  preferences.auto_update_frequency=10;
  preferences.display_method=DISPLAY_METHOD_AUTO;
  preferences.receive_method=RECEIVE_METHOD_AUTO;
  strcpy(preferences.save_filename,"test.jpg");
  preferences.save_mode=SAVE_MODE_IMMEDIATE;
  preferences.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
  preferences.save_period=1;
  strcpy(preferences.ftp_address,"ftp.sf.net");
  strcpy(preferences.ftp_user,"user_string");
  strcpy(preferences.ftp_password,"password_string");
  strcpy(preferences.ftp_filename,"test.bmp");
  strcpy(preferences.ftp_path,"/path/");
  preferences.ftp_mode=FTP_MODE_IMMEDIATE;
  preferences.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
  preferences.ftp_period=1;
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
      //fprintf(stderr,"Parsing config...\n");
      ParseConfigFile(fd);
      fclose(fd);
      //fprintf(stderr,"Parsed config...\n");
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

  if (preferences.auto_update)
    fprintf(fd,"%s YES\n",preferences_features[1]);
  else
    fprintf(fd,"%s NO\n",preferences_features[1]);

  fprintf(fd,"%s %f\n",preferences_features[2],preferences.auto_update_frequency);

  switch (preferences.display_method)
    {
    case DISPLAY_METHOD_GDK:
	fprintf(fd,"%s GDK\n",preferences_features[3]);
	break;
    case DISPLAY_METHOD_XV:
	fprintf(fd,"%s XV\n",preferences_features[3]);
	break;
    case DISPLAY_METHOD_AUTO:
    default:
	fprintf(fd,"%s AUTO\n",preferences_features[3]);
	break;
    }

  switch (preferences.receive_method)
    {
    case RECEIVE_METHOD_RAW1394:
	fprintf(fd,"%s RAW1394\n",preferences_features[4]);
	break;
    case RECEIVE_METHOD_VIDEO1394:
	fprintf(fd,"%s VIDEO1394\n",preferences_features[4]);
	break;
    case RECEIVE_METHOD_AUTO:
    default:
	fprintf(fd,"%s AUTO\n",preferences_features[4]);
	break;
    }

  fprintf(fd,"%s %s\n",preferences_features[5],preferences.save_filename);

  switch (preferences.save_mode)
    {
    case SAVE_MODE_PERIODIC:
      fprintf(fd,"%s SAVE_MODE_PERIODIC\n",preferences_features[6]);
      break;
    case SAVE_MODE_IMMEDIATE:
    default:
      fprintf(fd,"%s SAVE_MODE_IMMEDIATE\n",preferences_features[6]);
      break;
    }

  switch (preferences.save_scratch)
    {
    case SAVE_SCRATCH_OVERWRITE:
      fprintf(fd,"%s SAVE_SCRATCH_OVERWRITE\n",preferences_features[7]);
      break;
    case SAVE_SCRATCH_SEQUENTIAL:
    default:
      fprintf(fd,"%s SAVE_SCRATCH_SEQUENTIAL\n",preferences_features[7]);
      break;
    }
      
  fprintf(fd,"%s %d\n",preferences_features[8],preferences.save_period);

  fprintf(fd,"%s %s\n",preferences_features[9],preferences.ftp_address);
  fprintf(fd,"%s %s\n",preferences_features[10],preferences.ftp_user);
  fprintf(fd,"%s %s\n",preferences_features[11],preferences.ftp_password);
  fprintf(fd,"%s %s\n",preferences_features[12],preferences.ftp_filename);
  fprintf(fd,"%s %s\n",preferences_features[13],preferences.ftp_path);

  switch (preferences.ftp_mode)
    {
    case FTP_MODE_PERIODIC:
      fprintf(fd,"%s FTP_MODE_PERIODIC\n",preferences_features[14]);
      break;
    case FTP_MODE_IMMEDIATE:
    default:
      fprintf(fd,"%s FTP_MODE_IMMEDIATE\n",preferences_features[14]);
      break;
    }

  switch (preferences.ftp_scratch)
    {
    case FTP_SCRATCH_OVERWRITE:
      fprintf(fd,"%s FTP_SCRATCH_OVERWRITE\n",preferences_features[15]);
      break;
    case FTP_SCRATCH_SEQUENTIAL:
    default:
      fprintf(fd,"%s FTP_SCRATCH_SEQUENTIAL\n",preferences_features[15]);
      break;
    }
      
  fprintf(fd,"%s %d\n",preferences_features[16],preferences.ftp_period);

  fclose(fd);
}


void
ParseConfigFile(FILE* fd)
{

  char feature_string[256],feature_value[256];
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
	      needle=strstr(feature_value,"YES");
	      preferences.auto_update=(needle!=NULL);
	      break;
	    case AUTO_UPDATE_FREQUENCY:
	      preferences.auto_update_frequency=atof(feature_value);
	      break;
	    case DISPLAY_METHOD:
	      if(strstr(feature_value,"AUTO")!=NULL)
		preferences.display_method=DISPLAY_METHOD_AUTO;
	      else
		if(strstr(feature_value,"GDK")!=NULL)
		  preferences.display_method=DISPLAY_METHOD_GDK;
		else
		  if(strstr(feature_value,"XV")!=NULL)
		    preferences.display_method=DISPLAY_METHOD_XV;
		  else
		    MainError("Invalid display_method");
	      break;
	    case RECEIVE_METHOD:
	      if(strstr(feature_value,"AUTO")!=NULL)
		preferences.receive_method=RECEIVE_METHOD_AUTO;
	      else
		if(strstr(feature_value,"RAW1394")!=NULL)
		  preferences.receive_method=RECEIVE_METHOD_RAW1394;
		else
		  if(strstr(feature_value,"VIDEO1394")!=NULL)
		    preferences.receive_method=RECEIVE_METHOD_VIDEO1394;
		  else
		    MainError("Invalid receive_method");
	      break;
	    case SAVE_FILENAME:
	      strcpy(preferences.save_filename,feature_value);
	      break;
	    case SAVE_MODE:
	      if(strstr(feature_value,"SAVE_MODE_IMMEDIATE")!=NULL)
		preferences.save_mode=SAVE_MODE_IMMEDIATE;
	      else
		if(strstr(feature_value,"SAVE_MODE_PERIODIC")!=NULL)
		  preferences.save_mode=SAVE_MODE_PERIODIC;
		else
		  MainError("Invalid save_mode");
	      break;
	    case SAVE_SCRATCH:
	      if(strstr(feature_value,"SAVE_SCRATCH_OVERWRITE")!=NULL)
		preferences.save_scratch=SAVE_SCRATCH_OVERWRITE;
	      else
		if(strstr(feature_value,"SAVE_SCRATCH_SEQUENTIAL")!=NULL)
		  preferences.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
		else
		  MainError("Invalid save_scratch option");
	      break;
	    case SAVE_PERIOD:
	      preferences.save_period=atoi(feature_value);
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
	    case FTP_MODE:
	      if(strstr(feature_value,"FTP_MODE_IMMEDIATE")!=NULL)
		preferences.ftp_mode=FTP_MODE_IMMEDIATE;
	      else
		if(strstr(feature_value,"FTP_MODE_PERIODIC")!=NULL)
		  preferences.ftp_mode=FTP_MODE_PERIODIC;
		else
		  MainError("Invalid ftp_mode");
	      break;
	    case FTP_SCRATCH:
	      if(strstr(feature_value,"FTP_SCRATCH_OVERWRITE")!=NULL)
		preferences.ftp_scratch=FTP_SCRATCH_OVERWRITE;
	      else
		if(strstr(feature_value,"FTP_SCRATCH_SEQUENTIAL")!=NULL)
		  preferences.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
		  else
		    MainError("Invalid ftp_scratch option");
	      break;
	    case FTP_PERIOD:
	      preferences.ftp_period=atoi(feature_value);
	      break;
	    default:
	      MainError("Invalid config item");
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

  out=(char*)malloc(256*sizeof(char));

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
