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

#ifdef HAVE_FTPLIB
#include <ftplib.h>
#endif 

#include <pthread.h>
#include <libdc1394/dc1394_control.h>
#include "thread_base.h"
#include "thread_ftp.h"
#include "definitions.h"
#include "preferences.h"
#include "tools.h"

extern PrefsInfo preferences;
extern int current_camera;
 
gint
FtpStartThread(void)
{
  chain_t* ftp_service=NULL;
  ftpthread_info_t *info=NULL;
  gchar *tmp;

  ftp_service=GetService(SERVICE_FTP,current_camera);

  if (ftp_service==NULL)// if no FTP service running...
    {
      //fprintf(stderr,"No FTP service found, inserting new one\n");
      ftp_service=(chain_t*)malloc(sizeof(chain_t));
      ftp_service->data=(void*)malloc(sizeof(ftpthread_info_t));
      info=(ftpthread_info_t*)ftp_service->data;
      pthread_mutex_init(&ftp_service->mutex_data, NULL);
      pthread_mutex_init(&ftp_service->mutex_struct, NULL);
      pthread_mutex_init(&info->mutex_cancel_ftp, NULL);

      /* if you want a clean-interrupt thread:*/
      pthread_mutex_lock(&info->mutex_cancel_ftp);
      info->cancel_ftp_req=0;
      pthread_mutex_unlock(&info->mutex_cancel_ftp);

      /* setup ftp_thread: handles, ...*/
      pthread_mutex_lock(&ftp_service->mutex_data);
      info->period=preferences.ftp_period;
      info->counter=0;
      strcpy(info->address, preferences.ftp_address);
      strcpy(info->user, preferences.ftp_user);
      strcpy(info->password, preferences.ftp_password);
      strcpy(info->path, preferences.ftp_path);
      strcpy(info->filename, preferences.ftp_filename);
      tmp = strrchr(info->filename, '.');
      
      if (tmp==NULL)
	{
	  MainError("You should supply an extension");
	  pthread_mutex_unlock(&ftp_service->mutex_data);
	  FreeChain(ftp_service);
	  return(0);
	}

      tmp[0] = '\0';// cut filename before point
      strcpy(info->filename_ext, strrchr(preferences.ftp_filename, '.'));

      CommonChainSetup(ftp_service,SERVICE_FTP,current_camera);
      info->imlib_buffer_size=ftp_service->width*ftp_service->height*3;
      info->ftp_buffer=(unsigned char*)malloc(info->imlib_buffer_size*sizeof(unsigned char));

      info->ftp_scratch=preferences.ftp_scratch;

#ifdef HAVE_FTPLIB
      if (!OpenFtpConnection(info))
	{
	  MainError("Failed to open FTP connection");
	  pthread_mutex_unlock(&ftp_service->mutex_data);
	  FreeChain(ftp_service);
	  return(0);
	}
#else
      MainError("You don't have FTPLIB");
      pthread_mutex_unlock(&ftp_service->mutex_data);
      FreeChain(ftp_service);
      return(0);
#endif
      pthread_mutex_unlock(&ftp_service->mutex_data);

      /* Insert chain and start service*/
      pthread_mutex_lock(&ftp_service->mutex_struct);
      InsertChain(ftp_service,current_camera);
      pthread_mutex_unlock(&ftp_service->mutex_struct);

      pthread_mutex_lock(&ftp_service->mutex_data);
      pthread_mutex_lock(&ftp_service->mutex_struct);
      if (pthread_create(&ftp_service->thread, NULL,
			 FtpThread,(void*) ftp_service))
	  {
	    /* error starting thread. You should cleanup here
	       (free, unset global vars,...):*/

	    /* Mendatory cleanups:*/
	    RemoveChain(ftp_service,current_camera);
	    pthread_mutex_unlock(&ftp_service->mutex_struct);
	    pthread_mutex_unlock(&ftp_service->mutex_data);
	    free(info->ftp_buffer);
	    FreeChain(ftp_service);
	    return(-1);
	  }
      pthread_mutex_unlock(&ftp_service->mutex_struct);
      pthread_mutex_unlock(&ftp_service->mutex_data);
      
    }

  return (1);
}


void*
FtpCleanupThread(void* arg) 
{
  chain_t* ftp_service;
  ftpthread_info_t *info;

  ftp_service=(chain_t*)arg;
  info=(ftpthread_info_t*)ftp_service->data;
  /* Specific cleanups: */

  /* Mendatory cleanups: */
  pthread_mutex_unlock(&ftp_service->mutex_data);
  FtpStopThread(); // we do this in case of auto-kill from the thread.
}

void*
FtpThread(void* arg)
{
  static gchar filename_out[STRING_SIZE];
  chain_t* ftp_service=NULL;
  ftpthread_info_t *info=NULL;
  GdkImlibImage *im=NULL;
  long int skip_counter;
  char tmp_string[20];
  float delay;

  ftp_service=(chain_t*)arg;
  pthread_mutex_lock(&ftp_service->mutex_data);
  info=(ftpthread_info_t*)ftp_service->data;
  skip_counter=(info->period-1); /* send immediately, then start skipping */
  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&ftp_service->mutex_data);
 

  // time inits:
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

  while (1)
    { 
      /* Clean cancel handlers */
      pthread_mutex_lock(&info->mutex_cancel_ftp);
      if (info->cancel_ftp_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_ftp);
	  return ((void*)1);
	}
      else
	{
	  pthread_mutex_unlock(&info->mutex_cancel_ftp);
	  pthread_mutex_lock(&ftp_service->mutex_data);
	  if(RollBuffers(ftp_service)) // have buffers been rolled?
	    {
	      if (skip_counter==(info->period-1))
		{
		  skip_counter=0;
		  convert_to_rgb(ftp_service->current_buffer, info->ftp_buffer,
				 ftp_service->mode, ftp_service->width,
				 ftp_service->height, ftp_service->format7_color_mode,
				 ftp_service->bayer, ftp_service->bpp);
		  if (info->ftp_scratch == FTP_SCRATCH_OVERWRITE)
		    {
		      sprintf(filename_out, "%s%s", info->filename,info->filename_ext);
		    }
		  else
		    if (info->ftp_scratch == FTP_SCRATCH_SEQUENTIAL)
		      {
			sprintf(filename_out, "%s_%10.10li%s", info->filename,
				info->counter++, info->filename_ext);
		      }

		  im=gdk_imlib_create_image_from_data(info->ftp_buffer, NULL, ftp_service->width, ftp_service->height);
#ifdef HAVE_FTPLIB
		  if (!CheckFtpConnection(info))
		    {
		      MainError("Ftp connection lost for good");
		      // AUTO CANCEL THREAD
		      pthread_mutex_lock(&info->mutex_cancel_ftp);
		      info->cancel_ftp_req=1;
		      pthread_mutex_unlock(&info->mutex_cancel_ftp);
		    }
		  else
		    {
		      FtpPutFrame(filename_out, im, info);
		    }
#endif
		  if (im != NULL)
		    gdk_imlib_kill_image(im);
		}
	      else
		skip_counter++;

      info->current_time=times(&info->tms_buf);
      delay=(float)(info->current_time-info->prev_time)/CLK_TCK;
      info->frames++;
      if (delay>1.0) // update every second
	{
	  sprintf(tmp_string," %.2f",(float)info->frames/delay);
	  //fprintf(stderr,"ftp: %s fps\n",tmp_string);
	  /*
	  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"fps_ftp"),
			       ctxt.fps_ftp_ctxt, ctxt.fps_ftp_id);
	  ctxt.fps_ftp_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"fps_ftp"),
						 ctxt.fps_ftp_ctxt, tmp_string);
	  */
	  info->prev_time=info->current_time;
	  info->frames=0;
	}

	      pthread_mutex_unlock(&ftp_service->mutex_data);
	    }
	  else
	    {
	      pthread_mutex_unlock(&ftp_service->mutex_data);
	      usleep(THREAD_LOOP_SLEEP_TIME_US);
	    }
	}
    }
}


gint
FtpStopThread(void)
{
  ftpthread_info_t *info;
  chain_t *ftp_service;
  ftp_service=GetService(SERVICE_FTP,current_camera);

  if (ftp_service!=NULL)// if FTP service running...
    {
      info=(ftpthread_info_t*)ftp_service->data;
      /* Clean cancel handler: */
      pthread_mutex_lock(&info->mutex_cancel_ftp);
      info->cancel_ftp_req=1;
      pthread_mutex_unlock(&info->mutex_cancel_ftp);

      /* common handlers...*/
      pthread_join(ftp_service->thread, NULL);

      pthread_mutex_lock(&ftp_service->mutex_data);
      pthread_mutex_lock(&ftp_service->mutex_struct);
      RemoveChain(ftp_service,current_camera);

      /* Do custom cleanups here...*/
      free(info->ftp_buffer);
#ifdef HAVE_FTPLIB
      CloseFtpConnection(info->ftp_handle);
#endif
      /* Mendatory cleanups: */
      pthread_mutex_unlock(&ftp_service->mutex_struct);
      pthread_mutex_unlock(&ftp_service->mutex_data);
      FreeChain(ftp_service);

    }

  return (1);
}

#ifdef HAVE_FTPLIB
gboolean OpenFtpConnection(ftpthread_info_t* info)
{
  char  tmp[STRING_SIZE];

  FtpInit();

  MainStatus("Ftp: starting...\n");
  if (!FtpConnect(info->address, &info->ftp_handle))
    {
      MainError("Ftp: connection to server failed");
      return FALSE;
    }

  if (FtpLogin(info->user, info->password, info->ftp_handle) != 1)    {
      MainError("Ftp: login failed.");
      return FALSE;
    }

  sprintf(tmp, "Ftp: logged in as %s", info->user);
  MainStatus(tmp);
  
  if (info->path != NULL && strcmp(info->path,""))
    {
      if (!FtpChdir(info->path, info->ftp_handle))
	{
	  MainError("Ftp: chdir failed");
	  return FALSE;
	}
      sprintf(tmp, "Ftp: chdir %s", info->path);
      MainStatus(tmp);
    }

  MainStatus("Ftp: ready to send");

  return TRUE;
}

void
CloseFtpConnection(netbuf *ftp_handle)
{
  FtpQuit(ftp_handle);
}

gboolean
CheckFtpConnection(ftpthread_info_t* info)
{
 
  if (!FtpChdir(".", info->ftp_handle))
    // we can't access the current directory! Connection is probably lost. Reconnect: 
    if (!OpenFtpConnection(info))
      {
	MainError("Ftp: Can't restore lost connection");
	return FALSE;
      }
  return TRUE;
}

gboolean
FtpPutFrame(char *filename, GdkImlibImage *im, ftpthread_info_t* info)
{
  //netbuf **file_handle=NULL;
  char tmp[STRING_SIZE];


  // we have to write to a local tmp file to convert...
  
  sprintf(tmp,"/tmp/coriander_ftp_image%s",info->filename_ext);

  gdk_imlib_save_image(im, tmp, NULL);

  if (!FtpPut(tmp, filename, FTPLIB_IMAGE, info->ftp_handle))
    {
      MainError("Ftp failed to put file.");
      return FALSE;
    }

  return TRUE;
}

#endif // HAVE_FTPLIB
