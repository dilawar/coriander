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

#include <pthread.h>
#include <libdc1394/dc1394_control.h>
#include "thread_base.h"
#include "thread_save.h" 
#include "definitions.h"
#include "preferences.h"
#include "tools.h"

extern PrefsInfo preferences;
extern int current_camera;

gint
SaveStartThread(void)
{
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  gchar *tmp;

  save_service=GetService(SERVICE_SAVE,current_camera);

  if (save_service==NULL)// if no SAVE service running...
    {
      //fprintf(stderr,"No SAVE service found, inserting new one\n");
      save_service=(chain_t*)malloc(sizeof(chain_t));
      save_service->data=(void*)malloc(sizeof(savethread_info_t));
      info=(savethread_info_t*)save_service->data;
      pthread_mutex_init(&save_service->mutex_data, NULL);
      pthread_mutex_init(&save_service->mutex_struct, NULL);
      pthread_mutex_init(&info->mutex_cancel_save, NULL);

      /* if you want a clean-interrupt thread:*/
      pthread_mutex_lock(&info->mutex_cancel_save);
      info->cancel_save_req=0;
      pthread_mutex_unlock(&info->mutex_cancel_save);

      /* setup save_thread: handles, ...*/
      pthread_mutex_lock(&save_service->mutex_data);
      strcpy(info->filename, preferences.save_filename);
      tmp = strrchr(info->filename, '.');
      
      if (tmp==NULL)
	{
	  MainError("You should supply an extension");
	  pthread_mutex_unlock(&save_service->mutex_data);
	  FreeChain(save_service);
	  return(0);
	}

      tmp[0] = '\0';// cut filename before point
      strcpy(info->filename_ext, strrchr(preferences.save_filename, '.'));

      info->period=preferences.save_period;
      CommonChainSetup(save_service,SERVICE_SAVE,current_camera);
  
      info->save_buffer=(unsigned char*)malloc(save_service->width*save_service->height*3
						*sizeof(unsigned char));

      info->counter=0;
      info->save_scratch=preferences.save_scratch;

      pthread_mutex_unlock(&save_service->mutex_data);

      /* Insert chain and start service*/
      pthread_mutex_lock(&save_service->mutex_struct);
      InsertChain(save_service,current_camera);
      pthread_mutex_unlock(&save_service->mutex_struct);

      pthread_mutex_lock(&save_service->mutex_data);
      pthread_mutex_lock(&save_service->mutex_struct);
      if (pthread_create(&save_service->thread, NULL,
			 SaveThread,(void*) save_service))
	  {
	    /* error starting thread. You should cleanup here
	       (free, unset global vars,...):*/

	    /* Mendatory cleanups:*/
	    RemoveChain(save_service,current_camera);
	    pthread_mutex_unlock(&save_service->mutex_struct);
	    pthread_mutex_unlock(&save_service->mutex_data);
	    free(info->save_buffer);
	    FreeChain(save_service);
	    return(-1);
	  }
      pthread_mutex_unlock(&save_service->mutex_struct);
      pthread_mutex_unlock(&save_service->mutex_data);
      
    }
  //fprintf(stderr," SAVE service started\n");

  return (1);
}


void*
SaveCleanupThread(void* arg) 
{
  chain_t* save_service;
  savethread_info_t *info;

  save_service=(chain_t*)arg;
  info=(savethread_info_t*)save_service->data;
  /* Specific cleanups: */

  /* Mendatory cleanups: */
  pthread_mutex_unlock(&save_service->mutex_data);
}

void*
SaveThread(void* arg)
{
  static gchar filename_out[256];
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  GdkImlibImage *im=NULL;
  long int skip_counter;

  save_service=(chain_t*)arg;
  pthread_mutex_lock(&save_service->mutex_data);
  info=(savethread_info_t*)save_service->data;
  skip_counter=0;

  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&save_service->mutex_data);

  while (1)
    { 
      /* Clean cancel handlers */
      pthread_mutex_lock(&info->mutex_cancel_save);
      if (info->cancel_save_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_save);
	  return ((void*)1);
	}
      else
	{
	  pthread_mutex_unlock(&info->mutex_cancel_save);
	  pthread_mutex_lock(&save_service->mutex_data);
	  if(RollBuffers(save_service)) // have buffers been rolled?
	    {
	      if (skip_counter==(info->period-1))
		{
		  skip_counter=0;
		  convert_to_rgb(save_service->current_buffer, info->save_buffer,
				 save_service->mode, save_service->width,
				 save_service->height);
		  if (info->save_scratch == SAVE_SCRATCH_OVERWRITE)
		    {
		      sprintf(filename_out, "%s%s", info->filename,info->filename_ext);
		    }
		  else
		    if (info->save_scratch == SAVE_SCRATCH_SEQUENTIAL)
		      {
			sprintf(filename_out, "%s_%10.10li%s", info->filename,
				info->counter++, info->filename_ext);
		      }
		  
		  im=gdk_imlib_create_image_from_data(info->save_buffer,NULL,
						      save_service->width, save_service->height);
		  gdk_imlib_save_image(im, filename_out, NULL);
		  if (im != NULL) gdk_imlib_kill_image(im);
		}
	      else
		skip_counter++;
	      pthread_mutex_unlock(&save_service->mutex_data);
	    }
	  else
	    {
	      pthread_mutex_unlock(&save_service->mutex_data);
	      usleep(THREAD_LOOP_SLEEP_TIME_US);
	    }
	}
    }
}


gint
SaveStopThread(void)
{
  savethread_info_t *info;
  chain_t *save_service;
  save_service=GetService(SERVICE_SAVE,current_camera);

  if (save_service!=NULL)// if SAVE service running...
    {
      //fprintf(stderr,"SAVE service found, stopping\n");
      info=(savethread_info_t*)save_service->data;
      /* Clean cancel handler: */
      pthread_mutex_lock(&info->mutex_cancel_save);
      info->cancel_save_req=1;
      pthread_mutex_unlock(&info->mutex_cancel_save);

      /* common handlers...*/
      pthread_join(save_service->thread, NULL);

      pthread_mutex_lock(&save_service->mutex_data);
      pthread_mutex_lock(&save_service->mutex_struct);
      RemoveChain(save_service,current_camera);

      /* Do custom cleanups here...*/
      free(info->save_buffer);
      
      /* Mendatory cleanups: */
      pthread_mutex_unlock(&save_service->mutex_struct);
      pthread_mutex_unlock(&save_service->mutex_data);
      FreeChain(save_service);

      //fprintf(stderr," SAVE service stopped\n");
    }

  return (1);
}
