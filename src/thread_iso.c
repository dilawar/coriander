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
#include "thread_iso.h"
#include "preferences.h"
#include "topology.h"
#include "definitions.h"
#include "tools.h" 

extern dc1394_miscinfo *misc_info;
extern dc1394_camerainfo *camera;
extern SelfIdPacket_t *selfid;
extern PrefsInfo preferences; 
extern int current_camera;

gint IsoStartThread(void)
{
  int maxspeed;
  chain_t* iso_service=NULL;
  isothread_info_t *info=NULL;

  iso_service=GetService(SERVICE_ISO,current_camera);

  if (iso_service==NULL)// if no ISO service running...
    {
      //fprintf(stderr,"No ISO service found, inserting new one\n");
      iso_service=(chain_t*)malloc(sizeof(chain_t));
      iso_service->data=(void*)malloc(sizeof(isothread_info_t));
      pthread_mutex_init(&iso_service->mutex_struct, NULL);
      pthread_mutex_init(&iso_service->mutex_data, NULL);
      pthread_mutex_lock(&iso_service->mutex_data);

      info=(isothread_info_t*)iso_service->data;

      /* currently FORMAT_STILL_IMAGE is not supported*/
      if (misc_info->format == FORMAT_STILL_IMAGE)
	{
	  FreeChain(iso_service);
	  pthread_mutex_unlock(&iso_service->mutex_data);
	  return(-1);
	}
      
      info->handle = NULL;
      
      // the iso receive handler gets its own raw1394 handle to free the controls
      if ( (info->handle = dc1394_create_handle(0)) < 0)
	{
	  FreeChain(iso_service);
	  pthread_mutex_unlock(&iso_service->mutex_data);
	  return(-1);
	}

      switch (selfid->packetZero.phySpeed)
	{
	case 1: maxspeed=SPEED_200;break;
	case 2: maxspeed=SPEED_400;break;
	default: maxspeed=SPEED_100;break;
	}
      
      switch(preferences.receive_method)
	{
	case RECEIVE_METHOD_VIDEO1394:
	  if (dc1394_dma_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				       misc_info->format, misc_info->mode, maxspeed,
				       misc_info->framerate, DMA_BUFFERS, &info->capture)
	      == DC1394_SUCCESS)
	    {
	      info->receive_method=RECEIVE_METHOD_VIDEO1394;
	    }
	  else
	    {
	      MainError("Can't use VIDEO1394. Try AUTO receive mode.");
	      raw1394_destroy_handle(info->handle);
	      pthread_mutex_unlock(&iso_service->mutex_data);
	      FreeChain(iso_service);
	      return(-1);
	    }
	  break;
	case RECEIVE_METHOD_RAW1394:
	  if (dc1394_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				    misc_info->format, misc_info->mode, maxspeed,
				    misc_info->framerate, &info->capture)
	      == DC1394_SUCCESS)
	    {
	      info->receive_method=RECEIVE_METHOD_RAW1394;
	    }
	  else
	    {
	      MainError("Can't use RAW1394. Try AUTO receive mode.");
	      raw1394_destroy_handle(info->handle);
	      pthread_mutex_unlock(&iso_service->mutex_data);
	      FreeChain(iso_service);
	      return(-1);
	    }
	  break;
	  /*	case RECEIVE_METHOD_AUTO:
	  if (dc1394_dma_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
					misc_info->format, misc_info->mode, maxspeed,
					misc_info->framerate, DMA_BUFFERS, &info->capture)
	      == DC1394_SUCCESS)
	    {
	      //fprintf(stderr,"Auto video1394 selection\n");
	      info->receive_method=RECEIVE_METHOD_VIDEO1394;
	    }
	  else
	    if (dc1394_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				     misc_info->format, misc_info->mode, maxspeed,
				     misc_info->framerate, &info->capture)
		== DC1394_SUCCESS)
	      {
		//fprintf(stderr,"Auto raw1394 selection\n");
		info->receive_method=RECEIVE_METHOD_RAW1394;
	      }
	    else
	      {
		MainError("Can't find receive method.");
		raw1394_destroy_handle(info->handle);
		pthread_mutex_unlock(&iso_service->mutex_data);
		FreeChain(iso_service);
		//fprintf(stderr," Could not start ISO\n");
		return(-1);
	      }
	  */
	}
      //fprintf(stderr," 1394 setup OK\n");


      CommonChainSetup(iso_service, SERVICE_ISO, current_camera);
      pthread_mutex_unlock(&iso_service->mutex_data);
      
      pthread_mutex_lock(&iso_service->mutex_struct);
      InsertChain(iso_service,current_camera);
      pthread_mutex_unlock(&iso_service->mutex_struct);

      pthread_mutex_lock(&iso_service->mutex_data);
      pthread_mutex_lock(&iso_service->mutex_struct);
      if (pthread_create(&iso_service->thread, NULL, IsoThread,(void*) iso_service))
	{
	  RemoveChain(iso_service,current_camera);
	  pthread_mutex_unlock(&iso_service->mutex_struct);
	  pthread_mutex_unlock(&iso_service->mutex_data);
	  FreeChain(iso_service);
	  return(-1);
	}
      else
	pthread_mutex_unlock(&iso_service->mutex_struct);
	pthread_mutex_unlock(&iso_service->mutex_data);

	//fprintf(stderr," ISO thread started\n");
    }

  return (1);
}

void*
IsoCleanupThread(void* arg) 
{
  chain_t* iso_service;
  isothread_info_t *info;

  iso_service=(chain_t*)arg;
  info=(isothread_info_t*)iso_service->data;

  if ((info->receive_method == RECEIVE_METHOD_VIDEO1394))
    {
      //dc1394_dma_done_with_buffer(&info->capture);
      // this should be done at the condition it has not been executed before or
      // else we get an ioctl error from libdc1394. How to do this, I don't know...
      //fprintf(stderr,"dma done with buffer\n");
    }
  

    pthread_mutex_unlock(&iso_service->mutex_data);

  
  return(NULL);

}

void*
IsoThread(void* arg)
{
  chain_t *iso_service;
  isothread_info_t *info;

  // we should only use mutex_data in this function

  iso_service=(chain_t*)arg;
  pthread_mutex_lock(&iso_service->mutex_data);
  info=(isothread_info_t*)iso_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  pthread_mutex_unlock(&iso_service->mutex_data);

  while (1)
    {
      pthread_testcancel();
      pthread_cleanup_push((void*)IsoCleanupThread, (void*)iso_service);
      if (info->receive_method == RECEIVE_METHOD_RAW1394)
	dc1394_single_capture(info->handle, &info->capture);
      else
	{
	  dc1394_dma_single_capture(&info->capture);
	  dc1394_dma_done_with_buffer(&info->capture);
	}
      memcpy(iso_service->current_buffer,
	     info->capture.capture_buffer,
	     iso_service->bytes_per_frame);
      pthread_mutex_lock(&iso_service->mutex_data);
      RollBuffers(iso_service);
      pthread_mutex_unlock(&iso_service->mutex_data);
      pthread_cleanup_pop(0);
    }
}


gint IsoStopThread(void)
{
  isothread_info_t *info;
  chain_t *iso_service;
  iso_service=GetService(SERVICE_ISO,current_camera);  

  if (iso_service!=NULL)// if ISO service running...
    {
      //fprintf(stderr,"ISO service found, stopping\n");
      info=(isothread_info_t*)iso_service->data;
      pthread_cancel(iso_service->thread);
      pthread_join(iso_service->thread, NULL);
      pthread_mutex_lock(&iso_service->mutex_data);
      pthread_mutex_lock(&iso_service->mutex_struct);
      RemoveChain(iso_service,current_camera);

      if (info->receive_method == RECEIVE_METHOD_VIDEO1394)
	{
	  dc1394_dma_release_camera(info->handle, &info->capture);
	  dc1394_dma_unlisten(camera->handle, &info->capture);
	}
      else 
	dc1394_release_camera(camera->handle, &info->capture);
      
      raw1394_destroy_handle(info->handle);
      info->handle = NULL;

      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);

      FreeChain(iso_service);
      //fprintf(stderr," ISO service stopped\n");

    }

  return (1);
}
