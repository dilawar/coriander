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

#include "thread_iso.h"

extern PrefsInfo preferences; 
extern GtkWidget *main_window;
extern CtxtInfo ctxt;
extern camera_t* camera;

gint IsoStartThread(camera_t* cam)
{
  int maxspeed;
  chain_t* iso_service=NULL;
  isothread_info_t *info=NULL;

  iso_service=GetService(cam, SERVICE_ISO);

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
    if (cam->misc_info.format == FORMAT_STILL_IMAGE) {
      FreeChain(iso_service);
      return(-1);
    }
    
    info->handle = NULL;
    
    // the iso receive handler gets its own raw1394 handle to free the controls
    if ( (info->handle = dc1394_create_handle(0)) < 0) { // THIS PORT FIXED TO ZERO IS WRONG !!!!!!!!!!!!!!!!!!!!!!!!
      FreeChain(iso_service);
      return(-1);
    }
    
    switch (cam->selfid.packetZero.phySpeed) {
    case 1: maxspeed=SPEED_200;break;
    case 2: maxspeed=SPEED_400;break;
    default: maxspeed=SPEED_100;break;
    }
    //fprintf(stderr,"    Setting up capture\n");

    // copy params if we are the current camera
    if (cam==camera) {
      info->receive_method=preferences.receive_method;
      strcpy(info->video1394_device, preferences.video1394_device);
      info->capture.dma_device_file=info->video1394_device;
      info->video1394_dropframes=preferences.video1394_dropframes;
      info->dma_buffer_size=preferences.dma_buffer_size;
    }

    switch(info->receive_method) {
    case RECEIVE_METHOD_VIDEO1394:
      if (cam->misc_info.format!=FORMAT_SCALABLE_IMAGE_SIZE)
	if (dc1394_dma_setup_capture(cam->camera_info.handle, cam->camera_info.id, cam->misc_info.iso_channel, 
				     cam->misc_info.format, cam->misc_info.mode, maxspeed,
				     cam->misc_info.framerate, info->dma_buffer_size,
				     info->video1394_dropframes,
				     info->capture.dma_device_file, &info->capture)
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
	if (dc1394_dma_setup_format7_capture(cam->camera_info.handle, cam->camera_info.id, cam->misc_info.iso_channel, 
					     cam->misc_info.mode, maxspeed, QUERY_FROM_CAMERA,
					     QUERY_FROM_CAMERA, QUERY_FROM_CAMERA,
					     QUERY_FROM_CAMERA, QUERY_FROM_CAMERA, 
					     info->dma_buffer_size, &info->capture)
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
      if (cam->misc_info.format!=FORMAT_SCALABLE_IMAGE_SIZE)
	if (dc1394_setup_capture(cam->camera_info.handle, cam->camera_info.id, cam->misc_info.iso_channel, 
				 cam->misc_info.format, cam->misc_info.mode, maxspeed,
				 cam->misc_info.framerate, &info->capture)
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
	if (dc1394_setup_format7_capture(cam->camera_info.handle, cam->camera_info.id, cam->misc_info.iso_channel, 
					 cam->misc_info.mode, maxspeed, QUERY_FROM_CAMERA,
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
    CommonChainSetup(cam, iso_service, SERVICE_ISO);
    // init image buffers structs
    info->temp=NULL;
    info->temp_size=0;
    info->temp_allocated=0;

    //pthread_mutex_unlock(&iso_service->mutex_data);
    
    pthread_mutex_lock(&iso_service->mutex_struct);
    InsertChain(cam,iso_service);
    //pthread_mutex_unlock(&iso_service->mutex_struct);
    
    //pthread_mutex_lock(&iso_service->mutex_data);
    //pthread_mutex_lock(&iso_service->mutex_struct);
    if (pthread_create(&iso_service->thread, NULL, IsoThread,(void*) iso_service)) {
      RemoveChain(cam, iso_service);
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

  if ((info->receive_method == RECEIVE_METHOD_VIDEO1394)) {
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
  float tmp, fps;
  char *tmp_string;

  tmp_string=(char*)malloc(20*sizeof(char));

  iso_service=(chain_t*)data;
  info=(isothread_info_t*)iso_service->data;

  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=fabs(0.0);
  else
    fps=fabs((float)info->frames/tmp);
  
  sprintf(tmp_string," %.2f",fps);
  
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_receive"),
		       ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
  ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_receive"),
					 ctxt.fps_receive_ctxt, tmp_string);
  
  pthread_mutex_lock(&iso_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&iso_service->mutex_data);

  free(tmp_string);

  return 1;
}

void*
IsoThread(void* arg)
{
  chain_t *iso_service;
  isothread_info_t *info;
  int dma_ok=DC1394_FAILURE;

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

  while (1) {
    pthread_testcancel();
    pthread_cleanup_push((void*)IsoCleanupThread, (void*)iso_service);
    
    if (info->receive_method == RECEIVE_METHOD_RAW1394)
      dc1394_single_capture(info->handle, &info->capture);
    else
      dma_ok=dc1394_dma_single_capture(&info->capture);
    
    ftime(&info->rawtime);
    localtime_r(&info->rawtime.time, &(iso_service->current_buffer->captime));
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
    switch (iso_service->current_buffer->stereo_decoding) {
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
    switch (iso_service->current_buffer->bayer) {
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
    case NO_BAYER_DECODING:
      //fprintf(stderr,"memcopy\n");
      // this is only necessary if no stereo was performed
      if (iso_service->current_buffer->stereo_decoding==NO_STEREO_DECODING)
	memcpy(iso_service->current_buffer->image, info->temp,
	       iso_service->current_buffer->bytes_per_frame);
      break;
    }
    
    // FPS computation:
    info->current_time=times(&info->tms_buf);
    info->frames++;
    
    if ((info->receive_method == RECEIVE_METHOD_VIDEO1394)&&(dma_ok==DC1394_SUCCESS))
      dc1394_dma_done_with_buffer(&info->capture);
    
    pthread_mutex_unlock(&iso_service->mutex_data);
    
    pthread_mutex_lock(&iso_service->mutex_data);
    RollBuffers(iso_service);
    pthread_mutex_unlock(&iso_service->mutex_data);
    pthread_cleanup_pop(0);
  }

}


gint IsoStopThread(camera_t* cam)
{
  isothread_info_t *info;
  chain_t *iso_service;
  iso_service=GetService(cam,SERVICE_ISO);  

  if (iso_service!=NULL) { // if ISO service running...
    //fprintf(stderr,"ISO service found, stopping\n");
    info=(isothread_info_t*)iso_service->data;
    pthread_cancel(iso_service->thread);
    pthread_join(iso_service->thread, NULL);
    pthread_mutex_lock(&iso_service->mutex_data);
    pthread_mutex_lock(&iso_service->mutex_struct);
    
    gtk_timeout_remove(info->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_receive"), ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_receive"), ctxt.fps_receive_ctxt, "");
    
    RemoveChain(cam,iso_service);
    
    if ((info->temp!=NULL)&&(info->temp_allocated>0)) {
      free(info->temp);
      //fprintf(stderr,"temp freed\n");
      info->temp=NULL;
      info->temp_allocated=0;
      info->temp_size=0;
    }
    
    if (info->receive_method == RECEIVE_METHOD_VIDEO1394) {
      dc1394_dma_unlisten(cam->camera_info.handle, &info->capture);
      dc1394_dma_release_camera(info->handle, &info->capture);
    }
    else 
      dc1394_release_camera(cam->camera_info.handle, &info->capture);
    
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
  info=(isothread_info_t*)iso_service->data;
  // copy harmless parameters anyway:
  pthread_mutex_lock(&iso_service->camera->uimutex);

  iso_service->current_buffer->bpp=iso_service->camera->bpp;
  iso_service->current_buffer->bayer_pattern=iso_service->camera->bayer_pattern;
  iso_service->current_buffer->width=info->capture.frame_width;
  iso_service->current_buffer->height=info->capture.frame_height;
  iso_service->current_buffer->bytes_per_frame=info->capture.quadlets_per_frame*4;
  iso_service->current_buffer->mode=iso_service->camera->misc_info.mode;
  iso_service->current_buffer->format=iso_service->camera->misc_info.format;
  iso_service->current_buffer->format7_color_mode=iso_service->camera->format7_info.mode[iso_service->camera->misc_info.mode-MODE_FORMAT7_MIN].color_coding_id;
  iso_service->current_buffer->stereo_decoding=iso_service->camera->stereo;
  iso_service->current_buffer->bayer=iso_service->camera->bayer;
  info->orig_sizex=iso_service->current_buffer->width;
  info->orig_sizey=iso_service->current_buffer->height;

  if (iso_service->current_buffer->format!=FORMAT_SCALABLE_IMAGE_SIZE) {
    info->cond16bit=((iso_service->current_buffer->mode==MODE_640x480_MONO16)||
		     (iso_service->current_buffer->mode==MODE_800x600_MONO16)||
		     (iso_service->current_buffer->mode==MODE_1024x768_MONO16)||
		     (iso_service->current_buffer->mode==MODE_1280x960_MONO16)||
		     (iso_service->current_buffer->mode==MODE_1600x1200_MONO16));
  }
  else {
    // warning: little change, might have big effect
    info->cond16bit=(iso_service->camera->format7_info.mode[iso_service->current_buffer->mode-MODE_FORMAT7_MIN].color_coding_id==COLOR_FORMAT7_MONO16);
  }

  // the buffer sizes. If a size is not good, re-allocate.
  switch (iso_service->current_buffer->bayer) {
  case BAYER_DECODING_DOWNSAMPLE:
    switch (iso_service->current_buffer->stereo_decoding) {
    case STEREO_DECODING_FIELD:
    case STEREO_DECODING_INTERLACED:
      // height is the same (/2 downsampling, *2 stereo decoding)
      iso_service->current_buffer->width/=2;
      iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height*3; // RGB buffer -> *3
      AllocImageBuffer(iso_service);
      AllocTempBuffer(info->orig_sizey*info->orig_sizex*2*sizeof(unsigned char),info);
      break;
    case NO_STEREO_DECODING:
      iso_service->current_buffer->width/=2;
      iso_service->current_buffer->height/=2;
      iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height*3; // RGB buffer -> *3
      AllocImageBuffer(iso_service);
      if (info->cond16bit!=0)
	AllocTempBuffer(info->orig_sizey*info->orig_sizex*sizeof(unsigned char),info);
      else {
	AllocTempBuffer(0,info);
	info->temp=(unsigned char *)info->capture.capture_buffer;
      }
      break;
    }
    break;
  case BAYER_DECODING_EDGE_SENSE:
  case BAYER_DECODING_NEAREST:
    switch (iso_service->current_buffer->stereo_decoding) {
    case STEREO_DECODING_FIELD:
    case STEREO_DECODING_INTERLACED:
      iso_service->current_buffer->height*=2;
      iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height*3; // RGB buffer -> *3
      AllocImageBuffer(iso_service);
      AllocTempBuffer(info->orig_sizey*info->orig_sizex*2*sizeof(unsigned char),info);
      break;
    case NO_STEREO_DECODING:
      iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height*3; // RGB buffer -> *3
      AllocImageBuffer(iso_service);
      if (info->cond16bit!=0)
	AllocTempBuffer(info->orig_sizey*info->orig_sizex*sizeof(unsigned char),info);
      else {
	AllocTempBuffer(0,info);
	info->temp=(unsigned char *)info->capture.capture_buffer;
      }
      break;
    }
    break;
  case NO_BAYER_DECODING:
    switch (iso_service->current_buffer->stereo_decoding) {
    case STEREO_DECODING_FIELD:
    case STEREO_DECODING_INTERLACED:
      iso_service->current_buffer->height*=2;
      iso_service->current_buffer->bytes_per_frame=iso_service->current_buffer->width*iso_service->current_buffer->height; // GRAY output only (8bpp)
      AllocImageBuffer(iso_service);
      AllocTempBuffer(0,info);
      //fprintf(stderr,"info->temp linked to curent image buffer\n");
      // it is very important to allocate the image buffer before the following
      // assignation because the image allocation might change the buffer pointer! 
      info->temp=(unsigned char *)iso_service->current_buffer->image;
      break;
    case NO_STEREO_DECODING:
      AllocImageBuffer(iso_service);
      AllocTempBuffer(0,info);
      info->temp=(unsigned char *)info->capture.capture_buffer;
      break;
    }
    break;
  }
  /*
  fprintf(stderr,"Buffer sizes: temp=%lld image=%lld\n",info->temp_size,iso_service->current_buffer->bytes_per_frame);


  if (iso_service->current_buffer->image==NULL)
    fprintf(stderr,"Normal buffer not allocated. Segfault expected!\n");
  if (info->temp==NULL)
    fprintf(stderr,"Temp buffer nor allocated nor set. Segfault expected!\n");
  */
  SetColorMode(iso_service->current_buffer);
  
  pthread_mutex_unlock(&iso_service->camera->uimutex);

}

void
AllocTempBuffer(long long unsigned int requested_size, isothread_info_t* info) 
{
  if (requested_size==0) {
    if (info->temp_allocated>0) {
      free(info->temp);
      //fprintf(stderr,"temp freed\n");
    }
    info->temp_allocated=0;
    info->temp_size=0;
    //fprintf(stderr,"dummy temp used, no malloc\n");
    }
  else { // some allocated space is required
    if (requested_size!=info->temp_size) { // req and actual size don't not match
      if (info->temp_allocated>0) {
	free(info->temp);
	//fprintf(stderr,"temp freed\n");
      }
      info->temp=(unsigned char *)malloc(requested_size*sizeof(unsigned char));
      info->temp_allocated=1;
      info->temp_size=requested_size;
      //fprintf(stderr,"temp allocated with size %d at 0x%x for a resolution of %d x %d\n",temp_requested_size,info->temp,
      //  iso_service->current_buffer->width,iso_service->current_buffer->height);
    }
  }
}

void
AllocImageBuffer(chain_t* iso_service) 
{
  // ============ Allocate standard buffer ===============
  if (iso_service->current_buffer->buffer_size!=iso_service->current_buffer->bytes_per_frame) {
    // (re)allocation is necessary
    //fprintf(stderr,"(re-)allocation...\n");
    if (iso_service->current_buffer->image!=NULL) {
      free(iso_service->current_buffer->image);
      //fprintf(stderr,"image buffer freed\n");
      iso_service->current_buffer->image=NULL;
    }
    iso_service->current_buffer->image=(unsigned char*)malloc(iso_service->current_buffer->bytes_per_frame*sizeof(unsigned char));
    //fprintf(stderr,"0x%lx\n",(unsigned long int)iso_service->current_buffer->image);
    iso_service->current_buffer->buffer_size=iso_service->current_buffer->bytes_per_frame;
  }
}

void
SetColorMode(buffer_t *buffer)
{
  float bpp=-1;

  if (buffer->bayer==NO_BAYER_DECODING) {
    switch(buffer->mode) {
    case MODE_160x120_YUV444:
      buffer->buffer_color_mode=COLOR_FORMAT7_YUV444;
      bpp=3;
      break;
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      if (buffer->stereo_decoding!=NO_STEREO_DECODING) {
	buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	bpp=1;
      }
      else {
	buffer->buffer_color_mode=COLOR_FORMAT7_YUV422;
	bpp=2;
      }
      break;
    case MODE_640x480_YUV411:
      buffer->buffer_color_mode=COLOR_FORMAT7_YUV411;
      bpp=1.5;
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      buffer->buffer_color_mode=COLOR_FORMAT7_RGB8;
      bpp=3;
      break;
    case MODE_640x480_MONO:
    case MODE_800x600_MONO:
    case MODE_1024x768_MONO:
    case MODE_1280x960_MONO:
    case MODE_1600x1200_MONO:
      buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
      bpp=1;
      break;
    case MODE_640x480_MONO16:
    case MODE_800x600_MONO16:
    case MODE_1024x768_MONO16:
    case MODE_1280x960_MONO16:
    case MODE_1600x1200_MONO16:
      if (buffer->stereo_decoding!=NO_STEREO_DECODING) {
	buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	bpp=1;
      }
      else {
	buffer->buffer_color_mode=COLOR_FORMAT7_MONO16;
	bpp=2;
      }
      break;
    case MODE_FORMAT7_0:
    case MODE_FORMAT7_1:
    case MODE_FORMAT7_2:
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      if ((buffer->format7_color_mode==COLOR_FORMAT7_MONO16)&&(buffer->stereo_decoding!=NO_STEREO_DECODING)) {
	buffer->buffer_color_mode=COLOR_FORMAT7_MONO8;
	bpp=1;
      }
      else {
	buffer->buffer_color_mode=buffer->format7_color_mode;
	switch (buffer->format7_color_mode) {
	case COLOR_FORMAT7_MONO8:
	  bpp=1;
	  break;
	case COLOR_FORMAT7_MONO16:
	  bpp=2;
	  break;
	case COLOR_FORMAT7_RGB8:
	  bpp=3;
	  break;
	case COLOR_FORMAT7_RGB16:
	  bpp=6;
	  break;
	case COLOR_FORMAT7_YUV444:
	  bpp=3;
	  break;
	case COLOR_FORMAT7_YUV422:
	  bpp=2;
	  break;
	case COLOR_FORMAT7_YUV411:
	  bpp=1.5;
	  break;
	}
      }
      break;
    }
  }
  else  { // we force RGB mode
    buffer->buffer_color_mode=COLOR_FORMAT7_RGB8;
    bpp=3;
  }

  if (bpp==-1)
    fprintf(stderr,"ERROR: BPP is -1!!\n");

  buffer->buffer_image_bytes=(int)(buffer->width*buffer->height*bpp);
  //fprintf(stderr,"Color mode set to %d\n",buffer->buffer_color_mode);

}
