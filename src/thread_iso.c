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

#include <pthread.h>
#include <libdc1394/dc1394_control.h>
#include "thread_base.h"
#include "thread_iso.h"
#include "preferences.h"
#include "conversions.h"
#include "topology.h"
#include "definitions.h"
#include "tools.h" 

extern Format7Info *format7_info;
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
	  if (misc_info->format!=FORMAT_SCALABLE_IMAGE_SIZE)
	    if (dc1394_dma_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
					 misc_info->format, misc_info->mode, maxspeed,
					 misc_info->framerate, DMA_BUFFERS,
					 preferences.video1394_dropframes,
					 preferences.video1394_device, &info->capture)
		== DC1394_SUCCESS)
	      {
		info->receive_method=RECEIVE_METHOD_VIDEO1394;
	      }
	    else
	      {
		MainError("Can't use VIDEO1394. Try RAW1394 receive mode.");
		dc1394_destroy_handle(info->handle);
		pthread_mutex_unlock(&iso_service->mutex_data);
		FreeChain(iso_service);
		return(-1);
	      }
	  else {
	    info->capture.dma_device_file = preferences.video1394_device;
	    if (dc1394_dma_setup_format7_capture(camera->handle, camera->id, misc_info->iso_channel, 
						 misc_info->mode, maxspeed, QUERY_FROM_CAMERA,
						 QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
						 QUERY_FROM_CAMERA, QUERY_FROM_CAMERA, 
						 DMA_BUFFERS, &info->capture)
		== DC1394_SUCCESS)
	      {
		info->receive_method=RECEIVE_METHOD_VIDEO1394;
	      }
	    else
	      {
		MainError("Can't use VIDEO1394. Try RAW1394 receive mode.");
		dc1394_destroy_handle(info->handle);
		pthread_mutex_unlock(&iso_service->mutex_data);
		FreeChain(iso_service);
		return(-1);
	      }
	  }
	  break;
	case RECEIVE_METHOD_RAW1394:
	  if (misc_info->format!=FORMAT_SCALABLE_IMAGE_SIZE)
	    if (dc1394_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				     misc_info->format, misc_info->mode, maxspeed,
				     misc_info->framerate, &info->capture)
		== DC1394_SUCCESS)
	      {
		info->receive_method=RECEIVE_METHOD_RAW1394;
	      }
	    else
	      {
		MainError("Can't use RAW1394. Try VIDEO1394 receive mode.");
		dc1394_destroy_handle(info->handle);
		pthread_mutex_unlock(&iso_service->mutex_data);
		FreeChain(iso_service);
		return(-1);
	      }
	  else
	    if (dc1394_setup_format7_capture(camera->handle, camera->id, misc_info->iso_channel, 
					     misc_info->mode, maxspeed, QUERY_FROM_CAMERA,
					     QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
					     QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
					     &info->capture)
		== DC1394_SUCCESS)
	      {
		info->receive_method=RECEIVE_METHOD_RAW1394;
	      }
	    else
	      {
		MainError("Can't use RAW1394. Try VIDEO1394 receive mode.");
		dc1394_destroy_handle(info->handle);
		pthread_mutex_unlock(&iso_service->mutex_data);
		FreeChain(iso_service);
		return(-1);
	      }
	  break;
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
  unsigned char *temp;
  int cond16bit;
  int factor;
  // we should only use mutex_data in this function

  iso_service=(chain_t*)arg;

  pthread_mutex_lock(&iso_service->mutex_data);
  info=(isothread_info_t*)iso_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  pthread_mutex_unlock(&iso_service->mutex_data);

  if (iso_service->bayer==BAYER_DECODING_DOWNSAMPLE)
    factor=2;
  else
    factor=1;

  if (iso_service->format!=FORMAT_SCALABLE_IMAGE_SIZE)
    cond16bit=((iso_service->mode==MODE_640x480_MONO16)||
	       (iso_service->mode==MODE_800x600_MONO16)||
	       (iso_service->mode==MODE_1024x768_MONO16)||
	       (iso_service->mode==MODE_1280x960_MONO16)||
	       (iso_service->mode==MODE_1600x1200_MONO16));
  else
    cond16bit=(format7_info->mode[iso_service->mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_MONO16);
  
  if (iso_service->stereo_decoding==NO_STEREO_DECODING)
    {
      if (cond16bit>0)
	{
	  temp=(unsigned char*)malloc(iso_service->width*factor*iso_service->height*factor*sizeof(unsigned char));
	  //fprintf(stderr,"tmp size: %ld\n",iso_service->width*factor*iso_service->height*factor);
	}
      else
	temp=(unsigned char *)info->capture.capture_buffer;
    }
  else
    {
      temp=(unsigned char*)malloc(iso_service->width*factor*iso_service->height*factor*sizeof(unsigned char));
      //fprintf(stderr,"tmp size: %ld\n",iso_service->width*factor*iso_service->height*factor);
    }

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
      pthread_mutex_lock(&iso_service->mutex_data);
      
      if (iso_service->stereo_decoding==STEREO_DECODING)
	{
	  //fprintf(stderr,"decoding stereo image...");
	  StereoDecode((unsigned char *)info->capture.capture_buffer,temp,
		       iso_service->width*factor*iso_service->height*factor/2);
	  //fprintf(stderr,"done\n");
	}
      else
	if (iso_service->bayer!=NO_BAYER_DECODING)
	  if (cond16bit>0)
	    y162y((unsigned char *)info->capture.capture_buffer,temp,
		  iso_service->width*factor*iso_service->height*factor);

      //fprintf(stderr,"bayer decoding...");
      switch (iso_service->bayer)
	{
	case BAYER_DECODING_NEAREST:
	  BayerNearestNeighbor(temp, iso_service->current_buffer,
			       iso_service->width, iso_service->height, iso_service->bayer_pattern);
	  break;
	case BAYER_DECODING_EDGE_SENSE:
	  BayerEdgeSense(temp, iso_service->current_buffer,
			 iso_service->width, iso_service->height, iso_service->bayer_pattern);
	  break;
	case BAYER_DECODING_DOWNSAMPLE:
	  BayerDownsample(temp, iso_service->current_buffer,
			 iso_service->width*factor, iso_service->height*factor, iso_service->bayer_pattern);
	  break;
	default:
	  memcpy(iso_service->current_buffer,
		 info->capture.capture_buffer,
		 iso_service->bytes_per_frame);
	  break;
	}
      //fprintf(stderr,"done\n");
      pthread_mutex_unlock(&iso_service->mutex_data);
	  
      pthread_mutex_lock(&iso_service->mutex_data);
      RollBuffers(iso_service);
      pthread_mutex_unlock(&iso_service->mutex_data);
      pthread_cleanup_pop(0);
    }

  if (cond16bit>0)
    free(temp);

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
      
      dc1394_destroy_handle(info->handle);
      info->handle = NULL;

      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);

      FreeChain(iso_service);
      //fprintf(stderr," ISO service stopped\n");

    }

  return (1);
}
