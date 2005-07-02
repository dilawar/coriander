/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#include "coriander.h"

gint IsoStartThread(camera_t* cam)
{
  int maxspeed, err;
  //int channel, speed;
  chain_t* iso_service=NULL;
  isothread_info_t *info=NULL;

  iso_service=GetService(cam, SERVICE_ISO);

  if (iso_service==NULL) { // if no ISO service running...
    iso_service=(chain_t*)malloc(sizeof(chain_t));
    iso_service->current_buffer=NULL;
    iso_service->next_buffer=NULL;
    iso_service->data=(void*)malloc(sizeof(isothread_info_t));
    pthread_mutex_init(&iso_service->mutex_struct, NULL);
    pthread_mutex_init(&iso_service->mutex_data, NULL);
    
    info=(isothread_info_t*)iso_service->data;
    
    /* currently FORMAT_STILL_IMAGE is not supported*/
    if ((cam->camera_info.mode >= DC1394_MODE_FORMAT6_MIN) &&
        (cam->camera_info.mode <= DC1394_MODE_FORMAT6_MAX)) {
      FreeChain(iso_service);
      return(-1);
    }

    // ONLY IF LEGACY. OTHERWISE S800.
    switch (cam->selfid.packetZero.phySpeed) {
    case 0: maxspeed=DC1394_SPEED_100;break;
    case 1: maxspeed=DC1394_SPEED_200;break;
    case 2: maxspeed=DC1394_SPEED_400;break;
    case 3: maxspeed=DC1394_SPEED_800;break;
#if 0
    case 4: maxspeed=DC1394_SPEED_1600;break;
    case 5: maxspeed=DC1394_SPEED_3200;break;
#endif
    default:
      fprintf(stderr, "%s: unhandled phy speed %d\n", __FUNCTION__, cam->selfid.packetZero.phySpeed);
      maxspeed=DC1394_SPEED_100;
      break;
    }

    if (maxspeed >= DC1394_SPEED_800) {
      if (dc1394_set_operation_mode(&cam->camera_info, DC1394_OPERATION_MODE_1394B)!=DC1394_SUCCESS) {
	fprintf(stderr,"Can't set 1394B mode. Reverting to 400Mbps\n");
	maxspeed=DC1394_SPEED_400;
      }
    }

    // copy params if we are the current camera
    if (cam==camera) {
      info->receive_method=cam->prefs.receive_method;
      strcpy(info->video1394_device, cam->prefs.video1394_device);
      info->capture.dma_device_file=info->video1394_device;
      info->video1394_dropframes=cam->prefs.video1394_dropframes;
      info->dma_buffer_size=cam->prefs.dma_buffer_size;
    }

    switch(info->receive_method) {
    case RECEIVE_METHOD_VIDEO1394:
      if (!((cam->camera_info.mode >= DC1394_MODE_FORMAT7_MIN) &&
	    (cam->camera_info.mode <= DC1394_MODE_FORMAT7_MAX))) {
	err=dc1394_dma_setup_capture(&cam->camera_info, cam->camera_info.iso_channel, 
				     cam->camera_info.mode, maxspeed,
				     cam->camera_info.framerate, info->dma_buffer_size,
				     info->video1394_dropframes, 
				     info->capture.dma_device_file, &info->capture);
	if (err!=DC1394_SUCCESS){
	  eprint("Failed to setup DMA capture. Error code %d\n",err);
	  FreeChain(iso_service);
	  return(-1);
	}
	info->receive_method=RECEIVE_METHOD_VIDEO1394;
      }
      else {
	err=dc1394_dma_setup_format7_capture(&cam->camera_info, cam->camera_info.iso_channel, 
					     cam->camera_info.mode, maxspeed, DC1394_QUERY_FROM_CAMERA,
					     DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA,
					     DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA, 
					     info->dma_buffer_size,
					     info->video1394_dropframes, 
					     info->capture.dma_device_file, &info->capture);
	
	if (err!=DC1394_SUCCESS){
	  eprint("Failed to setup DMA Format_7 capture. Error code %d\n",err);
	  FreeChain(iso_service);
	  return(-1);
	}
	info->receive_method=RECEIVE_METHOD_VIDEO1394;
      }
      break;
    case RECEIVE_METHOD_RAW1394:
      if (!((cam->camera_info.mode >= DC1394_MODE_FORMAT7_MIN) &&
	    (cam->camera_info.mode <= DC1394_MODE_FORMAT7_MAX))) {
	err=dc1394_setup_capture(&cam->camera_info, cam->camera_info.iso_channel, 
				 cam->camera_info.mode, maxspeed,
				 cam->camera_info.framerate, &info->capture);
	
	if (err!=DC1394_SUCCESS){
	  eprint("Failed to setup RAW1394 capture. Error code %d\n",err);
	  FreeChain(iso_service);
	  return(-1);
	}
	info->receive_method=RECEIVE_METHOD_RAW1394;
      }
      else {
	err=dc1394_setup_format7_capture(&cam->camera_info, cam->camera_info.iso_channel, 
					 cam->camera_info.mode, maxspeed, DC1394_QUERY_FROM_CAMERA,
					 DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA,
					 DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA,
					 &info->capture);
	if (err!=DC1394_SUCCESS){
	  eprint("Failed to setup RAW1394 Format_7 capture. Error code %d\n",err);
	  FreeChain(iso_service);
	  return(-1);
	}
      }
      break;
    }
    
    pthread_mutex_lock(&iso_service->mutex_data);
    CommonChainSetup(cam, iso_service, SERVICE_ISO);
    // init image buffers structs
    info->temp=NULL;
    info->temp_size=0;
    info->temp_allocated=0;

    pthread_mutex_lock(&iso_service->mutex_struct);
    InsertChain(cam,iso_service);

    //iso_service->timeout_func_id=-1;
    if (pthread_create(&iso_service->thread, NULL, IsoThread,(void*) iso_service)) {
      RemoveChain(cam, iso_service);
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
      FreeChain(iso_service);
      return(-1);
    }
    else {
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
    }
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
 
void*
IsoThread(void* arg)
{
  chain_t *iso_service;
  isothread_info_t *info;
  int dma_ok=DC1394_FAILURE;
  float tmp;
  // we should only use mutex_data in this function

  iso_service=(chain_t*)arg;

  pthread_mutex_lock(&iso_service->mutex_data);
  info=(isothread_info_t*)iso_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  pthread_mutex_unlock(&iso_service->mutex_data);

  // time inits:
  iso_service->prev_time = times(&iso_service->tms_buf);
  iso_service->fps_frames=0;
  iso_service->processed_frames=0;

  while (1) {
    pthread_testcancel();
    pthread_cleanup_push((void*)IsoCleanupThread, (void*)iso_service);

    // IF FRAME USED (and we use no-drop mode)
    if (((iso_service->ready>0)&&(iso_service->camera->prefs.iso_nodrop>0))||
	(iso_service->camera->prefs.iso_nodrop==0)) {
    
      if (info->receive_method == RECEIVE_METHOD_RAW1394)
	dc1394_capture(&info->capture, 1);
      else
	dma_ok=dc1394_dma_capture(&info->capture, 1, DC1394_VIDEO1394_WAIT);
    
      //printf("Got frame\n");
  
      gettimeofday(&info->rawtime, NULL);
      localtime_r(&info->rawtime.tv_sec, &(iso_service->current_buffer->captime));
      iso_service->current_buffer->captime_usec=info->rawtime.tv_usec;
      sprintf(iso_service->current_buffer->captime_string,"%04d%02d%02d-%02d%02d%02d-%03d",
	      iso_service->current_buffer->captime.tm_year+1900,
	      iso_service->current_buffer->captime.tm_mon+1,
	      iso_service->current_buffer->captime.tm_mday,
	      iso_service->current_buffer->captime.tm_hour,
	      iso_service->current_buffer->captime.tm_min,
	      iso_service->current_buffer->captime.tm_sec,
	      iso_service->current_buffer->captime_usec/1000);
      
      pthread_mutex_lock(&iso_service->mutex_data);
      
      // check current buffer status
      IsoThreadCheckParams(iso_service);
      
      // Stereo decoding
      switch (iso_service->current_buffer->stereo_decoding) {
      case STEREO_DECODING_INTERLACED:
	dc1394_deinterlace_stereo((unsigned char *)info->capture.capture_buffer,info->temp,
				  info->orig_sizex*info->orig_sizey*2);
	break;
      case STEREO_DECODING_FIELD:
	memcpy(info->temp,(unsigned char *)info->capture.capture_buffer,info->orig_sizex*info->orig_sizey*2);
	break;
      case NO_STEREO_DECODING:
	if ((iso_service->current_buffer->bayer!=NO_BAYER_DECODING)&&(info->cond16bit!=0)) {
	  dc1394_MONO16_to_MONO8((unsigned char *)info->capture.capture_buffer,info->temp,
				 info->orig_sizex*info->orig_sizey, iso_service->current_buffer->bpp);
	}
	else {
	  // it is necessary to put this here and not in the thread init or IsoThreadCheckParams function because
	  // the buffer might change at every capture (typically when capture is too slow and buffering is performed)
	  info->temp=(unsigned char*)info->capture.capture_buffer;
	}
	break;
      }
      
      // Bayer decoding
      // TO BE UPGRADED FOR 16bit...
      if (iso_service->current_buffer->bayer!=NO_BAYER_DECODING) {
	dc1394_bayer_decoding_8bit(info->temp, iso_service->current_buffer->image,
				   iso_service->current_buffer->width, iso_service->current_buffer->height,
				   iso_service->current_buffer->bayer_pattern,iso_service->current_buffer->bayer);
      }
      else {
	// this is only necessary if no stereo was performed
	if (iso_service->current_buffer->stereo_decoding==NO_STEREO_DECODING) {
	  memcpy(iso_service->current_buffer->image, info->temp,
		 iso_service->current_buffer->bytes_per_frame);
	}
      }
      
      // FPS computation:
      iso_service->current_time=times(&iso_service->tms_buf);
      iso_service->fps_frames++;
      iso_service->processed_frames++;
      
      tmp=(float)(iso_service->current_time-iso_service->prev_time)/sysconf(_SC_CLK_TCK);
      if (tmp==0)
	iso_service->fps=fabs(0.0);
      else
	iso_service->fps=fabs((float)iso_service->fps_frames/tmp);
      
      if ((info->receive_method == RECEIVE_METHOD_VIDEO1394)&&(dma_ok==DC1394_SUCCESS))
	dc1394_dma_done_with_buffer(&info->capture);
      
      PublishBufferForNext(iso_service);
      //fprintf(stderr,"Buffer soon rolled in ISO\n");
      pthread_mutex_unlock(&iso_service->mutex_data);
    }
    else
      usleep(0);
    //fprintf(stderr,"got frame %.7f\n",iso_service->fps);

    pthread_cleanup_pop(0);
  }
  
}


gint IsoStopThread(camera_t* cam)
{
  isothread_info_t *info;
  chain_t *iso_service;
  iso_service=GetService(cam,SERVICE_ISO);  

  if (iso_service!=NULL) { // if ISO service running...
    info=(isothread_info_t*)iso_service->data;
    pthread_cancel(iso_service->thread);
    pthread_join(iso_service->thread, NULL);
    pthread_mutex_lock(&iso_service->mutex_data);
    pthread_mutex_lock(&iso_service->mutex_struct);

    //eprint("test1\n");

    RemoveChain(cam,iso_service);
    
    if ((info->temp!=NULL)&&(info->temp_allocated>0)) {
      free(info->temp);
      info->temp=NULL;
      info->temp_allocated=0;
      info->temp_size=0;
    }
    //eprint("test2\n");
    
    if (info->receive_method == RECEIVE_METHOD_VIDEO1394) {
      dc1394_dma_unlisten(&info->capture);
      dc1394_dma_release_capture(&info->capture);
    }
    else 
      dc1394_release_capture(&info->capture);
    
    //eprint("test3\n");
    pthread_mutex_unlock(&iso_service->mutex_struct);
    pthread_mutex_unlock(&iso_service->mutex_data);
    
    FreeChain(iso_service);
    
  }
  //eprint("test final\n");
  
  return (1);
}


void
IsoThreadCheckParams(chain_t *iso_service)
{
  int bayer_ok, stereo_ok;
  int temp;
  isothread_info_t *info;
  info=(isothread_info_t*)iso_service->data;
  // copy harmless parameters anyway:
  pthread_mutex_lock(&iso_service->camera->uimutex);

  iso_service->current_buffer->bpp=iso_service->camera->bpp;
  iso_service->current_buffer->bayer_pattern=iso_service->camera->bayer_pattern;
  iso_service->current_buffer->width=info->capture.frame_width;
  iso_service->current_buffer->height=info->capture.frame_height;
  iso_service->current_buffer->bytes_per_frame=info->capture.quadlets_per_frame*4;
  iso_service->current_buffer->stereo_decoding=iso_service->camera->stereo;
  iso_service->current_buffer->bayer=iso_service->camera->bayer;
  info->orig_sizex=iso_service->current_buffer->width;
  info->orig_sizey=iso_service->current_buffer->height;

  IsOptionAvailableWithFormat(&bayer_ok, &stereo_ok, &info->cond16bit);

  if (bayer_ok==0) {
    iso_service->current_buffer->bayer=NO_BAYER_DECODING;
  }
  if (stereo_ok==0) {
    iso_service->current_buffer->stereo_decoding=NO_STEREO_DECODING;
  }

  // the buffer sizes. If a size is not good, re-allocate.
  switch (iso_service->current_buffer->bayer) {
  case DC1394_BAYER_METHOD_DOWNSAMPLE:
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
      }
      break;
    }
    break;
  case DC1394_BAYER_METHOD_EDGESENSE:
  case DC1394_BAYER_METHOD_NEAREST:
  case DC1394_BAYER_METHOD_SIMPLE:
  case DC1394_BAYER_METHOD_BILINEAR:
  case DC1394_BAYER_METHOD_HQLINEAR:
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
      break;
    }
    break;
  }


  if ((iso_service->camera->camera_info.mode >= DC1394_MODE_FORMAT7_MIN) &&
      (iso_service->camera->camera_info.mode <= DC1394_MODE_FORMAT7_MAX)) {
    temp=iso_service->camera->format7_info.mode[iso_service->camera->camera_info.mode-DC1394_MODE_FORMAT7_MIN].color_coding_id;
  }
  else {
    temp=-1;
  }
  SetColorMode(iso_service->camera->camera_info.mode,iso_service->current_buffer,temp);
  /*
  fprintf(stderr,"S:[%d %d] BPF:%lli ColMode:%d\n",
	  iso_service->current_buffer->width, iso_service->current_buffer->height,
	  iso_service->current_buffer->bytes_per_frame,
	  iso_service->current_buffer->color_mode);
  */
  pthread_mutex_unlock(&iso_service->camera->uimutex);

}

void
AllocTempBuffer(long long unsigned int requested_size, isothread_info_t* info) 
{
  if (requested_size==0) {
    if (info->temp_allocated>0) {
      free(info->temp);
    }
    info->temp_allocated=0;
    info->temp_size=0;
    }
  else { // some allocated space is required
    if (requested_size!=info->temp_size) { // req and actual size don't not match
      if (info->temp_allocated>0) {
	free(info->temp);
      }
      info->temp=(unsigned char *)malloc(requested_size*sizeof(unsigned char));
      info->temp_allocated=1;
      info->temp_size=requested_size;
    }
  }
}

void
AllocImageBuffer(chain_t* iso_service) 
{
  // ============ Allocate standard buffer ===============
  if (iso_service->current_buffer->buffer_size!=iso_service->current_buffer->bytes_per_frame) {
    // (re)allocation is necessary
    if (iso_service->current_buffer->image!=NULL) {
      free(iso_service->current_buffer->image);
      iso_service->current_buffer->image=NULL;
    }
    iso_service->current_buffer->image=(unsigned char*)malloc(iso_service->current_buffer->bytes_per_frame*sizeof(unsigned char));
    iso_service->current_buffer->buffer_size=iso_service->current_buffer->bytes_per_frame;
  }
}

void
SetColorMode(int mode, buffer_t *buffer, int f7_color)
{
  float bpp=-1;

  if (buffer->bayer==NO_BAYER_DECODING) {
    switch(mode) {
    case DC1394_MODE_160x120_YUV444:
      buffer->color_mode=DC1394_COLOR_CODING_YUV444;
      break;
    case DC1394_MODE_320x240_YUV422:
    case DC1394_MODE_640x480_YUV422:
    case DC1394_MODE_800x600_YUV422:
    case DC1394_MODE_1024x768_YUV422:
    case DC1394_MODE_1280x960_YUV422:
    case DC1394_MODE_1600x1200_YUV422:
      if (buffer->stereo_decoding!=NO_STEREO_DECODING) {
	buffer->color_mode=DC1394_COLOR_CODING_MONO8;
      }
      else {
	buffer->color_mode=DC1394_COLOR_CODING_YUV422;
      }
      break;
    case DC1394_MODE_640x480_YUV411:
      buffer->color_mode=DC1394_COLOR_CODING_YUV411;
      break;
    case DC1394_MODE_640x480_RGB8:
    case DC1394_MODE_800x600_RGB8:
    case DC1394_MODE_1024x768_RGB8:
    case DC1394_MODE_1280x960_RGB8:
    case DC1394_MODE_1600x1200_RGB8:
      buffer->color_mode=DC1394_COLOR_CODING_RGB8;
      break;
    case DC1394_MODE_640x480_MONO8:
    case DC1394_MODE_800x600_MONO8:
    case DC1394_MODE_1024x768_MONO8:
    case DC1394_MODE_1280x960_MONO8:
    case DC1394_MODE_1600x1200_MONO8:
      buffer->color_mode=DC1394_COLOR_CODING_MONO8;
      break;
    case DC1394_MODE_640x480_MONO16:
    case DC1394_MODE_800x600_MONO16:
    case DC1394_MODE_1024x768_MONO16:
    case DC1394_MODE_1280x960_MONO16:
    case DC1394_MODE_1600x1200_MONO16:
      if (buffer->stereo_decoding!=NO_STEREO_DECODING) {
	buffer->color_mode=DC1394_COLOR_CODING_MONO8;
      }
      else {
	buffer->color_mode=DC1394_COLOR_CODING_MONO16;
      }
      break;
    case DC1394_MODE_FORMAT7_0:
    case DC1394_MODE_FORMAT7_1:
    case DC1394_MODE_FORMAT7_2:
    case DC1394_MODE_FORMAT7_3:
    case DC1394_MODE_FORMAT7_4:
    case DC1394_MODE_FORMAT7_5:
    case DC1394_MODE_FORMAT7_6:
    case DC1394_MODE_FORMAT7_7:
      if (f7_color==-1)
	fprintf(stderr,"ERROR: format7 asked but color mode is -1\n");
      if ((f7_color==DC1394_COLOR_CODING_MONO16)&&(buffer->stereo_decoding!=NO_STEREO_DECODING)) {
	buffer->color_mode=DC1394_COLOR_CODING_MONO8;
      }
      else {
	buffer->color_mode=f7_color;
      }
      break;
    }
  }
  else  { // we force RGB mode
    buffer->color_mode=DC1394_COLOR_CODING_RGB8;
  }

  dc1394_get_bytes_per_pixel(buffer->color_mode, &bpp);

  buffer->buffer_image_bytes=(int)((float)(buffer->width*buffer->height)*bpp);

  //fprintf(stderr,"%d\n",buffer->color_mode);
}
