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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <pthread.h>
#include <libdc1394/dc1394_control.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>
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
extern GtkWidget *commander_window;
extern int current_camera;
extern CtxtInfo ctxt;
extern uiinfo_t *uiinfo;

gint IsoStartThread(void)
{
  int maxspeed;
  chain_t* iso_service=NULL;
  isothread_info_t *info=NULL;

  iso_service=GetService(SERVICE_ISO,current_camera);

  if (iso_service==NULL) { // if no ISO service running...
    //fprintf(stderr,"*** No ISO service found, inserting new one\n");
    iso_service=(chain_t*)malloc(sizeof(chain_t));
    iso_service->current_buffer=NULL;
    iso_service->next_buffer=NULL;
    iso_service->data=(void*)malloc(sizeof(isothread_info_t));
    pthread_mutex_init(&iso_service->mutex_struct, NULL);
    pthread_mutex_init(&iso_service->mutex_data, NULL);
    
    info=(isothread_info_t*)iso_service->data;
    
    /* currently FORMAT_STILL_IMAGE is not supported*/
    if (misc_info->format == FORMAT_STILL_IMAGE) {
      FreeChain(iso_service);
      return(-1);
    }
    
    info->handle = NULL;
    
    // the iso receive handler gets its own raw1394 handle to free the controls
    if ( (info->handle = dc1394_create_handle(0)) < 0) {
      FreeChain(iso_service);
      return(-1);
    }
    
    switch (selfid->packetZero.phySpeed) {
    case 1: maxspeed=SPEED_200;break;
    case 2: maxspeed=SPEED_400;break;
    default: maxspeed=SPEED_100;break;
    }
    //fprintf(stderr,"    Setting up capture\n");
    switch(preferences.receive_method) {
    case RECEIVE_METHOD_VIDEO1394:
      if (misc_info->format!=FORMAT_SCALABLE_IMAGE_SIZE)
	if (dc1394_dma_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				     misc_info->format, misc_info->mode, maxspeed,
				     misc_info->framerate, DMA_BUFFERS,
				     preferences.video1394_dropframes,
				     preferences.video1394_device, &info->capture)
	    == DC1394_SUCCESS) {
	  info->receive_method=RECEIVE_METHOD_VIDEO1394;
	}
	else {
	  MainError("Failed to setup DMA capture with VIDEO1394");
	  dc1394_destroy_handle(info->handle);
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
	    == DC1394_SUCCESS) {
	  info->receive_method=RECEIVE_METHOD_VIDEO1394;
	}
	else {
	  MainError("Failed to setup Format_7 DMA capture with VIDEO1394");
	  dc1394_destroy_handle(info->handle);
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
	    == DC1394_SUCCESS) {
	  info->receive_method=RECEIVE_METHOD_RAW1394;
	}
	else {
	  MainError("Failed to setup capture with RAW1394");
	  dc1394_destroy_handle(info->handle);
	  FreeChain(iso_service);
	  return(-1);
	}
      else {
	if (dc1394_setup_format7_capture(camera->handle, camera->id, misc_info->iso_channel, 
					 misc_info->mode, maxspeed, QUERY_FROM_CAMERA,
					 QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
					 QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
					 &info->capture)
	    == DC1394_SUCCESS) {
	  info->receive_method=RECEIVE_METHOD_RAW1394;
	}
	else {
	  MainError("Failed to setup Format_7 capture with RAW1394");
	  dc1394_destroy_handle(info->handle);
	  FreeChain(iso_service);
		return(-1);
	}
      }
      break;
    }
    //fprintf(stderr,"   1394 setup OK\n");
    
    pthread_mutex_lock(&iso_service->mutex_data);
    CommonChainSetup(iso_service, SERVICE_ISO, current_camera);
    // init image buffers structs
    info->temp=NULL;
    info->temp_size=0;
    info->temp_allocated=0;

    pthread_mutex_unlock(&iso_service->mutex_data);
    
    pthread_mutex_lock(&iso_service->mutex_struct);
    InsertChain(iso_service,current_camera);
    pthread_mutex_unlock(&iso_service->mutex_struct);
    
    pthread_mutex_lock(&iso_service->mutex_data);
    pthread_mutex_lock(&iso_service->mutex_struct);
    if (pthread_create(&iso_service->thread, NULL, IsoThread,(void*) iso_service)) {
      RemoveChain(iso_service,current_camera);
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
      FreeChain(iso_service);
      return(-1);
    }
    else {
      info->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)IsoShowFPS, (gpointer*) iso_service);
      
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
    }
    //fprintf(stderr,"   ISO thread started\n");
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
  
  // clear timing info on GUI...
  pthread_mutex_unlock(&iso_service->mutex_data);
  
  return(NULL);

}
 
int
IsoShowFPS(gpointer *data)
{
  chain_t* iso_service;
  isothread_info_t *info;
  char tmp_string[20];
  float tmp, fps;

  iso_service=(chain_t*)data;
  info=(isothread_info_t*)iso_service->data;

  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=fabs(0.0);
  else
    fps=fabs((float)info->frames/tmp);
  
  sprintf(tmp_string," %.2f",fps);
  
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"fps_receive"),
		       ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
  ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"fps_receive"),
					 ctxt.fps_receive_ctxt, tmp_string);
  
  pthread_mutex_lock(&iso_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&iso_service->mutex_data);

  return 1;
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

  // time inits:
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

  while (1)
    {
      pthread_testcancel();
      pthread_cleanup_push((void*)IsoCleanupThread, (void*)iso_service);

      if (info->receive_method == RECEIVE_METHOD_RAW1394)
	dc1394_single_capture(info->handle, &info->capture);
      else
	dc1394_dma_single_capture(&info->capture);

      ftime(&info->rawtime);
      localtime_r(&info->rawtime.time, 
		  &(iso_service->current_buffer->captime));
      iso_service->current_buffer->captime.tm_year+=1900;
      iso_service->current_buffer->captime.tm_mon+=1;
      iso_service->current_buffer->captime_millisec=info->rawtime.millitm;
      sprintf(iso_service->current_buffer->captime_string,"%04d%02d%02d-%02d%02d%02d-%03d",
	      iso_service->current_buffer->captime.tm_year,
	      iso_service->current_buffer->captime.tm_mon,
	      iso_service->current_buffer->captime.tm_mday,
	      iso_service->current_buffer->captime.tm_hour,
	      iso_service->current_buffer->captime.tm_min,
	      iso_service->current_buffer->captime.tm_sec,
	      iso_service->current_buffer->captime_millisec);

      pthread_mutex_lock(&iso_service->mutex_data);
      
      // check current buffer status
      IsoThreadCheckParams(iso_service);

      // Stereo decoding
      switch (iso_service->current_buffer->stereo_decoding)
	{
	case STEREO_DECODING_INTERLACED:
	  StereoDecode((unsigned char *)info->capture.capture_buffer,info->temp,
		       info->orig_sizex*info->orig_sizey*2);
	  break;
	case STEREO_DECODING_FIELD:
	  memcpy(info->temp,(unsigned char *)info->capture.capture_buffer,info->orig_sizex*info->orig_sizey*2);
	  break;
	case NO_STEREO_DECODING:
	  if ((iso_service->current_buffer->bayer!=NO_BAYER_DECODING)&&(info->cond16bit!=0)) {
	    y162y((unsigned char *)info->capture.capture_buffer,info->temp,
		  info->orig_sizex*info->orig_sizey, iso_service->current_buffer->bpp);
	  }
	  break;
	}
      //fprintf(stderr,"0x%x\n",info->capture.capture_buffer);
      // Bayer decoding
      switch (iso_service->current_buffer->bayer)
	{
	case BAYER_DECODING_NEAREST:
	  BayerNearestNeighbor(info->temp, iso_service->current_buffer->image,
			       iso_service->current_buffer->width, iso_service->current_buffer->height, iso_service->current_buffer->bayer_pattern);
	  break;
	case BAYER_DECODING_EDGE_SENSE:
	  BayerEdgeSense(info->temp, iso_service->current_buffer->image,
			 iso_service->current_buffer->width, iso_service->current_buffer->height, iso_service->current_buffer->bayer_pattern);
	  break;
	case BAYER_DECODING_DOWNSAMPLE:
	  BayerDownsample(info->temp, iso_service->current_buffer->image,
			 iso_service->current_buffer->width, iso_service->current_buffer->height, iso_service->current_buffer->bayer_pattern);
	  break;
	default:
	  //fprintf(stderr,"memcopy\n");
	  memcpy(iso_service->current_buffer->image, info->temp,
		 iso_service->current_buffer->bytes_per_frame);
	      
	  break;
	}

      // FPS computation:
      info->current_time=times(&info->tms_buf);
      info->frames++;

      if (info->receive_method == RECEIVE_METHOD_VIDEO1394)
        dc1394_dma_done_with_buffer(&info->capture);
    
      pthread_mutex_unlock(&iso_service->mutex_data);
	  
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

      gtk_timeout_remove(info->timeout_func_id);
      gtk_statusbar_remove((GtkStatusbar*)lookup_widget(commander_window,"fps_receive"),
			   ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
      ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(commander_window,"fps_receive"),
					  ctxt.fps_receive_ctxt, "");

      RemoveChain(iso_service,current_camera);

      if ((info->temp!=NULL)&&(info->temp_allocated>0)) {
	free(info->temp);
	//fprintf(stderr,"temp freed\n");
	info->temp=NULL;
	info->temp_allocated=0;
	info->temp_size=0;
      }

      if (info->receive_method == RECEIVE_METHOD_VIDEO1394)
	{
	  dc1394_dma_unlisten(camera->handle, &info->capture);
	  dc1394_dma_release_camera(info->handle, &info->capture);
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


void
IsoThreadCheckParams(chain_t *iso_service)
{

  isothread_info_t *info;
  int change_detected=0;
  int temp_requested_size=0;
  info=(isothread_info_t*)iso_service->data;
  // copy harmless parameters anyway:
  pthread_mutex_lock(&uiinfo->mutex);
  iso_service->current_buffer->bpp=uiinfo->bpp;
  iso_service->current_buffer->bayer_pattern=uiinfo->bayer_pattern;

  if (iso_service->current_buffer->width==-1) {
    // we have to allocate something and get the parameters: it's the first pass:
    change_detected+=1;
  }
  // check sizes. This depends on Bayer decoding
  if (iso_service->current_buffer->bayer==BAYER_DECODING_DOWNSAMPLE) {
    
    // allow size to differ, this is normal operation.
    change_detected+=iso_service->current_buffer->width          !=(info->capture.frame_width)/2;
    if (iso_service->current_buffer->stereo_decoding!=NO_STEREO_DECODING){
      change_detected+=iso_service->current_buffer->height       !=(info->capture.frame_height);
    }
    else {
      change_detected+=iso_service->current_buffer->height       !=(info->capture.frame_height)/2;
    }
  }
  else {
    // size should be equal, or a change occured.
    change_detected+=iso_service->current_buffer->width          !=info->capture.frame_width;
    if (iso_service->current_buffer->stereo_decoding!=NO_STEREO_DECODING) {
      change_detected+=iso_service->current_buffer->height       !=info->capture.frame_height*2;
    }
    else {
      change_detected+=iso_service->current_buffer->height       !=info->capture.frame_height;
    }
  }
  // check mode and format:
  change_detected+=iso_service->current_buffer->mode!=misc_info->mode;
  change_detected+=iso_service->current_buffer->format!=misc_info->format;
  // check F7 color mode change
  change_detected+=((iso_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)&&
		    (iso_service->current_buffer->format7_color_mode!=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].color_coding_id));
  // check stereo decoding
  change_detected+=iso_service->current_buffer->stereo_decoding!=uiinfo->stereo;
  // check bayer decoding
  change_detected+=iso_service->current_buffer->bayer!=uiinfo->bayer;

  // (note: there is no check in bytes_per_frame as this mearly reflects some changes for other parameters)

  if (change_detected>0)
    {
      //fprintf(stderr,"Parameter change detected, updating... ------------------- \n");
      // copy all new parameters:
      iso_service->current_buffer->width=info->capture.frame_width;
      iso_service->current_buffer->height=info->capture.frame_height;
      iso_service->current_buffer->bytes_per_frame=info->capture.quadlets_per_frame*4;
      iso_service->current_buffer->mode=misc_info->mode;
      iso_service->current_buffer->format=misc_info->format;
      iso_service->current_buffer->format7_color_mode=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].color_coding_id;
      iso_service->current_buffer->stereo_decoding=uiinfo->stereo;
      iso_service->current_buffer->bayer=uiinfo->bayer;
      info->orig_sizex=iso_service->current_buffer->width;
      info->orig_sizey=iso_service->current_buffer->height;

      // CHECK TEMP BUFFER -----------------------------
      //fprintf(stderr," == TEMP ==\n");
      if (iso_service->current_buffer->format!=FORMAT_SCALABLE_IMAGE_SIZE)
	info->cond16bit=((iso_service->current_buffer->mode==MODE_640x480_MONO16)||
			 (iso_service->current_buffer->mode==MODE_800x600_MONO16)||
			 (iso_service->current_buffer->mode==MODE_1024x768_MONO16)||
			 (iso_service->current_buffer->mode==MODE_1280x960_MONO16)||
			 (iso_service->current_buffer->mode==MODE_1600x1200_MONO16));
      else
	info->cond16bit=(format7_info->mode[iso_service->current_buffer->mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_MONO16);
      
      if (iso_service->current_buffer->stereo_decoding!=NO_STEREO_DECODING)
	temp_requested_size=iso_service->current_buffer->width*iso_service->current_buffer->height*2*sizeof(unsigned char);
      else {
	if ((info->cond16bit!=0)&&(iso_service->current_buffer->bayer!=NO_BAYER_DECODING)) {
	  temp_requested_size=iso_service->current_buffer->width*iso_service->current_buffer->height*sizeof(unsigned char);
	}
	else
	  temp_requested_size=0;
      }

      if (temp_requested_size==0) {
	if (info->temp_allocated>0) {
	  free(info->temp);
	  //fprintf(stderr,"temp freed\n");
	}
	info->temp_allocated=0;
	info->temp_size=0;
	info->temp=(unsigned char *)info->capture.capture_buffer;
	//fprintf(stderr,"dummy temp used, no malloc\n");
      }
      else { // some allocated space is required
	if (temp_requested_size!=info->temp_size) { // req and actual size don't not match
	  if (info->temp_allocated>0) {
	    free(info->temp);
	    //fprintf(stderr,"temp freed\n");
	  }
	  info->temp=(unsigned char *)malloc(temp_requested_size*10);
	  info->temp_allocated=1;
	  info->temp_size=temp_requested_size;
	  //fprintf(stderr,"temp allocated with size %d at 0x%x for a resolution of %d x %d\n",temp_requested_size,info->temp,
	  //  iso_service->current_buffer->width,iso_service->current_buffer->height);
	}
      }

      // CHECK STANDARD BUFFER -------------------------
      //fprintf(stderr," == BUFFER ==\n");
      // if bayer decoding, we force a RGB24 buffer size
      if (iso_service->current_buffer->bayer!=NO_BAYER_DECODING) {
	iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height*3;
      }
      // if we use downsampling, buffer should be /4
      if (iso_service->current_buffer->bayer==BAYER_DECODING_DOWNSAMPLE) {
	iso_service->current_buffer->width/=2;
	iso_service->current_buffer->height/=2;
	iso_service->current_buffer->bytes_per_frame/=4;
      }
      // if it's stereo decoding, height is multiplied by 2
      if (iso_service->current_buffer->stereo_decoding!=NO_STEREO_DECODING) {
	iso_service->current_buffer->height*=2;
	// if we are no more in raw 16bit mode, we should *2. Otherwise, the factor is already in bytes_per_frame.
	if (iso_service->current_buffer->bayer!=NO_BAYER_DECODING) {
	  iso_service->current_buffer->bytes_per_frame*=2;
	}
      }

      if (iso_service->current_buffer->image!=NULL) {
	free(iso_service->current_buffer->image);
	iso_service->current_buffer->image=NULL;
      }

      iso_service->current_buffer->image=(unsigned char*)malloc(iso_service->current_buffer->bytes_per_frame*sizeof(unsigned char));
      //fprintf(stderr,"buffer allocated with size %d at 0x%x for a resolution of %d x %d\n",iso_service->current_buffer->bytes_per_frame,iso_service->current_buffer->image,
      //      iso_service->current_buffer->width,iso_service->current_buffer->height);

      if ((iso_service->current_buffer->image==NULL)||(info->temp==NULL))
	fprintf(stderr,"Can't allocate image buffers! Aiiie!\n");

      /*fprintf(stderr,"Final parameters:\n Size: %d x %d\n BPF: %ld\n Orig Size: %d x %d\n",
	      iso_service->current_buffer->width,iso_service->current_buffer->height,
	      iso_service->current_buffer->bytes_per_frame,
	      info->orig_sizex, info->orig_sizey);
      */
      SetColorMode(iso_service->current_buffer);

    }

  if (info->temp_allocated==0) {
    // temp not allocated. it is a dummy pointer to the capture buffer and must be updated anyway.
    info->temp=(unsigned char *)info->capture.capture_buffer;
  }

  pthread_mutex_unlock(&uiinfo->mutex);

}

void
SetColorMode(buffer_t *buffer)
{
   if (buffer->bayer==NO_BAYER_DECODING)
    {
      switch(buffer->mode)
	{
	case MODE_160x120_YUV444:
	  buffer->buffer_color_mode=COLOR_FORMAT7_YUV444;
	  break;
	case MODE_320x240_YUV422:
	case MODE_640x480_YUV422:
	case MODE_800x600_YUV422:
	case MODE_1024x768_YUV422:
	case MODE_1280x960_YUV422:
	case MODE_1600x1200_YUV422:
	  if (buffer->stereo_decoding!=NO_STEREO_DECODING)
	    buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	  else
	    buffer->buffer_color_mode=COLOR_FORMAT7_YUV422;
	  break;
	case MODE_640x480_YUV411:
	  buffer->buffer_color_mode=COLOR_FORMAT7_YUV411;
	  break;
	case MODE_640x480_RGB:
	case MODE_800x600_RGB:
	case MODE_1024x768_RGB:
	case MODE_1280x960_RGB:
	case MODE_1600x1200_RGB:
	  buffer->buffer_color_mode=COLOR_FORMAT7_RGB8;
	  break;
	case MODE_640x480_MONO:
	case MODE_800x600_MONO:
	case MODE_1024x768_MONO:
	case MODE_1280x960_MONO:
	case MODE_1600x1200_MONO:
	  buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	  break;
	case MODE_640x480_MONO16:
	case MODE_800x600_MONO16:
	case MODE_1024x768_MONO16:
	case MODE_1280x960_MONO16:
	case MODE_1600x1200_MONO16:
	  if (buffer->stereo_decoding!=NO_STEREO_DECODING)
	    buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	  else
	    buffer->buffer_color_mode=COLOR_FORMAT7_MONO16;
	  break;
	case MODE_FORMAT7_0:
	case MODE_FORMAT7_1:
	case MODE_FORMAT7_2:
	case MODE_FORMAT7_3:
	case MODE_FORMAT7_4:
	case MODE_FORMAT7_5:
	case MODE_FORMAT7_6:
	case MODE_FORMAT7_7:
	  if ((buffer->format7_color_mode==COLOR_FORMAT7_MONO16)&&(buffer->stereo_decoding!=NO_STEREO_DECODING))
	    buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	  else
	    buffer->buffer_color_mode=buffer->format7_color_mode;
	  break;
	}
    }
  else // we force RGB mode
    buffer->buffer_color_mode=COLOR_FORMAT7_RGB8;

   //fprintf(stderr,"Color mode set to %d\n",buffer->buffer_color_mode);

}
