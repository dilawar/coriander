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

gint
SaveStartThread(camera_t* cam)
{
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  gchar *tmp;

  save_service=GetService(camera, SERVICE_SAVE);

  if (save_service==NULL) { // if no SAVE service running...
    save_service=(chain_t*)malloc(sizeof(chain_t));
    save_service->current_buffer=NULL;
    save_service->next_buffer=NULL;
    save_service->data=(void*)malloc(sizeof(savethread_info_t));
    info=(savethread_info_t*)save_service->data;
    pthread_mutex_init(&save_service->mutex_data, NULL);
    pthread_mutex_init(&save_service->mutex_struct, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    /* if you want a clean-interrupt thread:*/
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* setup save_thread: handles, ...*/
    pthread_mutex_lock(&save_service->mutex_data);
    strcpy(info->filename, cam->prefs.save_basedir);

    // (JG) we should save the extension regardless of whether ImLib is doing conversion
    tmp = strrchr(info->filename, '.');
    if(tmp != NULL)
    {
      tmp[0] = '\0';// cut filename before point
      strcpy(info->filename_ext, strrchr(cam->prefs.save_basedir, '.'));
    }
    else { // no conversion
      strcpy(info->filename_ext, "");
    }

    // if we use conversion, look for the extension.
    if (cam->prefs.save_convert==SAVE_CONVERT_ON) {    
      if (tmp==NULL) {
	MainError("You should supply an extension");
	pthread_mutex_unlock(&save_service->mutex_data);
	FreeChain(save_service);
	return(-1);
      }
    }

    info->period=cam->prefs.save_period;
    CommonChainSetup(cam, save_service,SERVICE_SAVE);
    
    info->buffer=NULL;
    info->counter=0;
    info->scratch=cam->prefs.save_scratch;
    info->datenum=cam->prefs.save_datenum;
    // if format extension is ".raw", we dump raw data on the file and perform no conversion
    info->rawdump=cam->prefs.save_convert;
    info->use_ram_buffer=cam->prefs.use_ram_buffer;
    info->ram_buffer_size=cam->prefs.ram_buffer_size*1024*1024; // ram buffer size in MB

    info->bigbuffer=NULL;

    if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)) {
      info->bigbuffer_position=0;
      info->bigbuffer=(unsigned char*)malloc(info->ram_buffer_size*sizeof(unsigned char));
      if (info->bigbuffer==NULL) {
	MainError("Could not allocate memory for RAM buffer save service");
	pthread_mutex_unlock(&save_service->mutex_data);
	FreeChain(save_service);
	return(-1);
      }
    }

    /* Insert chain and start service*/
    pthread_mutex_lock(&save_service->mutex_struct);
    InsertChain(cam, save_service);
    if (pthread_create(&save_service->thread, NULL, SaveThread,(void*) save_service)) {
      /* error starting thread. You should cleanup here
	 (free, unset global vars,...):*/
      
      /* Mendatory cleanups:*/
      RemoveChain(cam,save_service);
      pthread_mutex_unlock(&save_service->mutex_struct);
      pthread_mutex_unlock(&save_service->mutex_data);
      free(info->buffer);
      FreeChain(save_service);
      return(-1);
    }

    pthread_mutex_unlock(&save_service->mutex_struct);
    pthread_mutex_unlock(&save_service->mutex_data);
    
  }
  
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

  SaveStopThread(save_service->camera);

  /* Mandatory cleanups: */
  //pthread_mutex_unlock(&save_service->mutex_data);

  return(NULL);
}


// TRUE if color mode is color, FALSE if MONO/GREYSCALE
int
isColor(int buffer_color_mode)
{
  switch(buffer_color_mode)  {
  case COLOR_FORMAT7_MONO8:
  case COLOR_FORMAT7_MONO16:
  case COLOR_FORMAT7_MONO16S:
  case COLOR_FORMAT7_RAW8:
  case COLOR_FORMAT7_RAW16:
    return(FALSE);
    break;
  case COLOR_FORMAT7_YUV411:
  case COLOR_FORMAT7_YUV422:
  case COLOR_FORMAT7_YUV444:
  case COLOR_FORMAT7_RGB8:
  case COLOR_FORMAT7_RGB16:
    return(TRUE);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(FALSE);
    break;
  }
  return(FALSE);
}


// return TRUE if format is not in PVN native RGB or GREYSCALE mode
int
needsConversionForPVN(int buffer_color_mode)
{
  switch(buffer_color_mode) {
  case COLOR_FORMAT7_YUV411:
  case COLOR_FORMAT7_YUV422:
  case COLOR_FORMAT7_YUV444:
    return(TRUE);
    break;
  case COLOR_FORMAT7_MONO8:
  case COLOR_FORMAT7_RGB8:
  case COLOR_FORMAT7_MONO16:
  case COLOR_FORMAT7_MONO16S:
  case COLOR_FORMAT7_RGB16:
  case COLOR_FORMAT7_RAW8:
  case COLOR_FORMAT7_RAW16:
    return(FALSE);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(FALSE);
    break;
  }
  return(FALSE);
}

// return Bytes in each channel after converted to RGB or MONO (YUV422 will convert to RGB8 thats why we need this extra function)
int
getConvertedBytesPerChannel(int buffer_color_mode)
{
  switch(buffer_color_mode) {
  case COLOR_FORMAT7_MONO8:
  case COLOR_FORMAT7_RAW8:
  case COLOR_FORMAT7_YUV411:
  case COLOR_FORMAT7_YUV422:
  case COLOR_FORMAT7_YUV444:
  case COLOR_FORMAT7_RGB8:
    return(1);
    break;
  case COLOR_FORMAT7_MONO16:
  case COLOR_FORMAT7_RAW16:
  case COLOR_FORMAT7_MONO16S:
  case COLOR_FORMAT7_RGB16:
    return(2);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(0);
    break;
  }
  return(0);
}


// return Bytes in each channel
int getBytesPerChannel(int buffer_color_mode)
{
  switch(buffer_color_mode) 
  {
    case COLOR_FORMAT7_MONO8:
    case COLOR_FORMAT7_RAW8:
    case COLOR_FORMAT7_YUV411:
    case COLOR_FORMAT7_YUV444:
    case COLOR_FORMAT7_RGB8:
      return(1);
      break;
    case COLOR_FORMAT7_YUV422:
    case COLOR_FORMAT7_MONO16:
    case COLOR_FORMAT7_RAW16:
    case COLOR_FORMAT7_MONO16S:
    case COLOR_FORMAT7_RGB16:
      return(2);
      break;
    default:
      fprintf(stderr, "Unknown buffer format!\n");
      return(0);
      break;
  }
  return(0);
}

// return Bytes in each pixel (3*bytes per channel if colour)
int
getBytesPerPixel(int buffer_color_mode)
{
  switch(buffer_color_mode) {
  case COLOR_FORMAT7_MONO8:
  case COLOR_FORMAT7_RAW8:
    return(1);
    break;
  case COLOR_FORMAT7_MONO16:
  case COLOR_FORMAT7_RAW16:
  case COLOR_FORMAT7_MONO16S:
    return(2);
    break;
  case COLOR_FORMAT7_YUV411:
  case COLOR_FORMAT7_YUV444:
  case COLOR_FORMAT7_RGB8:
    return(3);
    break;
  case COLOR_FORMAT7_YUV422:
  case COLOR_FORMAT7_RGB16:
    return(6);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(0);
    break;
  }
  return(0);
}

unsigned int
getDepth(unsigned long bufsize, int mode, unsigned int height, unsigned int width)
{
  int bytes_per_pixel=getBytesPerPixel(mode);
  return((unsigned int)bufsize/(bytes_per_pixel*height*width));
}

void
convert_for_pvn(unsigned char *buffer, unsigned int width, unsigned int height,
		unsigned int page, int buffer_color_mode, unsigned char *dest)
{
  unsigned char *buf_loc=buffer+page*getBytesPerPixel(buffer_color_mode)*width*height;

  if(dest==NULL)
    return;

  switch(buffer_color_mode) {
    case COLOR_FORMAT7_MONO8:
    case COLOR_FORMAT7_RAW8:
      memcpy(dest,buf_loc,width*height);
      break;
    case COLOR_FORMAT7_YUV411:
      uyyvyy2rgb(buf_loc,dest,width*height);
      break;
    case COLOR_FORMAT7_YUV422:
      uyvy2rgb(buf_loc,dest,width*height);
      break;
    case COLOR_FORMAT7_YUV444:
      uyv2rgb(buf_loc,dest,width*height);
      break;
    case COLOR_FORMAT7_RGB8:
      memcpy(dest,buf_loc,3*width*height);
      break;
    case COLOR_FORMAT7_MONO16:
    case COLOR_FORMAT7_MONO16S:
    case COLOR_FORMAT7_RAW16:
      memcpy(dest,buf_loc,2*width*height);
      break;
    case COLOR_FORMAT7_RGB16:
      memcpy(dest,buf_loc,6*width*height);
      break;
    default:
      fprintf(stderr, "Unknown buffer format!\n");
      break;
  }
}

// (JG) note: depth/# of pages is calculated from bufsize by dividing by # of bytes per image
void
writePVNHeader(FILE *fd, unsigned int mode, unsigned int height, unsigned int width,
	       unsigned int depth, unsigned int bpp, double framerate)
{
  unsigned int bytes_per_pixel;
  char magic[6];

  if(isColor(mode)==FALSE) //greyscale
    strcpy(magic,"PV5a");
  else // colour
    strcpy(magic,"PV6a");

  bytes_per_pixel=getBytesPerPixel(mode);
  fprintf(fd,"%s\n%d %d %d\n%d %f\n", magic, width, height, depth, bpp, framerate);
}

void*
SaveThread(void* arg)
{
  char *filename_out;
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  GdkImlibImage *im=NULL;
  long int skip_counter;
  FILE *fd=NULL;
  float tmp;
  int i;
  unsigned char* dest=NULL;

  filename_out=(char*)malloc(STRING_SIZE*sizeof(char));

  save_service=(chain_t*)arg;
  pthread_mutex_lock(&save_service->mutex_data);
  info=(savethread_info_t*)save_service->data;
  skip_counter=0;

  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&save_service->mutex_data);

  if (info->scratch==SAVE_SCRATCH_VIDEO) {
    sprintf(filename_out, "%s%s", info->filename,info->filename_ext);
    fd=fopen(filename_out,"w");
    if (fd==NULL)
      MainError("Can't create sequence file for saving");
    /*
    if (info->use_ram_buffer>0) {
      pthread_cleanup_push((void*)SaveCleanupThread, (void*) save_service);
    }
    */
  }
  
  // time inits:
  save_service->prev_time = times(&save_service->tms_buf);
  save_service->fps_frames=0;
  save_service->processed_frames=0;

  while (1) { 
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel);
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&save_service->mutex_data);
      if(RollBuffers(save_service)) { // have buffers been rolled?
	// check params
	SaveThreadCheckParams(save_service);
	if (save_service->current_buffer->width!=-1) {
	  if (skip_counter>=(info->period-1)) {
	    skip_counter=0;
	    // get filename
	    switch (info->scratch) {
            case SAVE_SCRATCH_VIDEO:
	      // (JG) if extension is PVN, write PVN header here
	      if ((strncasecmp(info->filename_ext, ".pvn",4)==0) && (info->counter==0) && (info->use_ram_buffer==FALSE)) {
	        info->counter++;
		/*	        fprintf(stderr,"File Size: %ld, Filename: %s\n", info->bigbuffer_position, info->filename);
				fprintf(stderr,"Image resolution: %d by %d, Bits Per Pixel (BPP): %d \n", 
				save_service->current_buffer->width, save_service->current_buffer->height,
				getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8);
				fprintf(stderr,"FPS: %f\n",save_service->fps); // FPS of most recent frame *** FIX THIS TO DO DEFAULT FPS *** */

	        writePVNHeader(fd, save_service->current_buffer->buffer_color_mode,
			       save_service->current_buffer->height,
			       save_service->current_buffer->width,
			       0, getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8, 0);
	      }
	      break;
	    case SAVE_SCRATCH_OVERWRITE:
	      sprintf(filename_out, "%s%s", info->filename,info->filename_ext);
	      break;
	    case SAVE_SCRATCH_SEQUENTIAL:
	      switch (info->datenum) {
	      case SAVE_TAG_DATE:
		sprintf(filename_out, "%s-%s%s", info->filename, save_service->current_buffer->captime_string, info->filename_ext);
		break;
	      case SAVE_TAG_NUMBER:
		sprintf(filename_out,"%s-%10.10li%s", info->filename, info->counter++, info->filename_ext);
		break;
	      }
	      break;
	    default:
	      break;
	    }
	    // rambuffer operation
	    if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)) {
	      if (info->ram_buffer_size-info->bigbuffer_position>=save_service->current_buffer->buffer_image_bytes) {
		memcpy(&info->bigbuffer[info->bigbuffer_position], save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes);
		info->bigbuffer_position+=save_service->current_buffer->buffer_image_bytes;
	      }
	      else { // buffer is full, exit thread
		info->cancel_req=1;
	      }
	    }
	    // normal operation
	    else {
	      if (info->rawdump>0) {
		if (info->scratch!=SAVE_SCRATCH_VIDEO) {
		  fd=fopen(filename_out,"w");
		  if (fd==NULL)
		    MainError("Can't create/open image file for saving");
		  else {
		    fwrite(save_service->current_buffer->image, 1, save_service->current_buffer->buffer_image_bytes, fd);
		    fclose(fd);
		  }
		}
		else { // video saving mode
		  if ( (strncasecmp(info->filename_ext, ".pvn",4)!=0) ||
		       (needsConversionForPVN(save_service->current_buffer->buffer_color_mode)==FALSE) ) {
		    fwrite(save_service->current_buffer->image, 1, save_service->current_buffer->buffer_image_bytes, fd);
		  }
		  else {
		    // we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
		    dest = (unsigned char*)malloc(3*save_service->current_buffer->width*save_service->current_buffer->height*sizeof(unsigned char));
		    convert_for_pvn(save_service->current_buffer->image, save_service->current_buffer->width,
				    save_service->current_buffer->height, 0, save_service->current_buffer->buffer_color_mode, dest);
                    fwrite(dest, 1, 3*save_service->current_buffer->width*save_service->current_buffer->height, fd);
                  }
		}
	      }
	      else {
		convert_to_rgb(save_service->current_buffer, info->buffer);
		im=gdk_imlib_create_image_from_data(info->buffer,NULL, save_service->current_buffer->width, save_service->current_buffer->height);
		if (im != NULL) {
		  if (gdk_imlib_save_image(im, filename_out, NULL)==0) {
		    MainError("Can't save image with Imlib!");
		  }
		  gdk_imlib_kill_image(im);
		}
		else {
		  MainError("Can't create gdk image!");
		}
	      }
	    }
	    save_service->fps_frames++;
	    save_service->processed_frames++;
	  }
	  else
	    skip_counter++;
	  
	  // FPS display
	  save_service->current_time=times(&save_service->tms_buf);
	  tmp=(float)(save_service->current_time-save_service->prev_time)/sysconf(_SC_CLK_TCK);
	  if (tmp==0)
	    save_service->fps=fabs(0.0);
	  else
	    save_service->fps=fabs((float)save_service->fps_frames/tmp);

	}
	pthread_mutex_unlock(&save_service->mutex_data);
      }
      else {
	pthread_mutex_unlock(&save_service->mutex_data);
      }
    }
    usleep(0);
  }
  if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)) {
    fwrite(info->bigbuffer, 1, info->bigbuffer_position, fd);
  }

  if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)&& (strncasecmp(info->filename_ext, ".pvn",4)==0)) {
    /*    fprintf(stderr,"File Size: %ld, Filename: %s\n", info->bigbuffer_position, info->filename);
	  fprintf(stderr,"Image resolution: %d by %d, Bits Per Pixel (BPP): %d \n", 
	  save_service->current_buffer->width, save_service->current_buffer->height,
	  getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8);
	  fprintf(stderr,"FPS: %f\n",save_service->fps); // FPS of most recent frame  */

    writePVNHeader(fd, save_service->current_buffer->buffer_color_mode,
		   save_service->current_buffer->height,
		   save_service->current_buffer->width,
		   getDepth(info->bigbuffer_position, save_service->current_buffer->buffer_color_mode, 
			    save_service->current_buffer->height, save_service->current_buffer->width),
		   getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8, 0);

    if(needsConversionForPVN(save_service->current_buffer->buffer_color_mode)==FALSE) {
      fwrite(info->bigbuffer, 1, info->bigbuffer_position, fd);
    }
    else {
      // we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
      dest = (unsigned char*)malloc(3*save_service->current_buffer->width*save_service->current_buffer->height*sizeof(unsigned char));
      
      for (i = 0; i < getDepth(info->bigbuffer_position, save_service->current_buffer->buffer_color_mode,
			       save_service->current_buffer->height, save_service->current_buffer->width); i++) {
        convert_for_pvn(info->bigbuffer, save_service->current_buffer->width,
			save_service->current_buffer->height, i, save_service->current_buffer->buffer_color_mode, dest);
        fwrite(dest, 1, 3*save_service->current_buffer->width*save_service->current_buffer->height, fd);
      }
    }
  }
  
  if ((info->scratch==SAVE_SCRATCH_VIDEO)&&(fd!=NULL)) {
    fclose(fd);
  }

  if (info->bigbuffer!=NULL) {
    free(info->bigbuffer);
  }

  if(dest != NULL)
    free(dest);

  pthread_mutex_unlock(&info->mutex_cancel);

  free(filename_out);
  return ((void*)1);
}


gint
SaveStopThread(camera_t* cam)
{
  savethread_info_t *info;
  chain_t *save_service;
  save_service=GetService(cam,SERVICE_SAVE);

  if (save_service!=NULL) { // if SAVE service running...
    info=(savethread_info_t*)save_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* common handlers...*/
    pthread_join(save_service->thread, NULL);
    
    pthread_mutex_lock(&save_service->mutex_data);
    pthread_mutex_lock(&save_service->mutex_struct);
    RemoveChain(cam,save_service);
    
    /* Do custom cleanups here...*/
    if (info->buffer!=NULL) {
      free(info->buffer);
      info->buffer=NULL;
    }
    
    /* Mendatory cleanups: */
    pthread_mutex_unlock(&save_service->mutex_struct);
    pthread_mutex_unlock(&save_service->mutex_data);
    FreeChain(save_service);
    
  }
  
  return (1);
}

void
SaveThreadCheckParams(chain_t *save_service)
{

  savethread_info_t *info;
  int buffer_size_change=0;
  info=(savethread_info_t*)save_service->data;

  // copy harmless parameters anyway:
  save_service->local_param_copy.bpp=save_service->current_buffer->bpp;
  save_service->local_param_copy.bayer_pattern=save_service->current_buffer->bayer_pattern;

  // if some parameters changed, we need to re-allocate the local buffers and restart the save
  if ((save_service->current_buffer->width!=save_service->local_param_copy.width)||
      (save_service->current_buffer->height!=save_service->local_param_copy.height)||
      (save_service->current_buffer->bytes_per_frame!=save_service->local_param_copy.bytes_per_frame)||
      (save_service->current_buffer->mode!=save_service->local_param_copy.mode)||
      (save_service->current_buffer->format!=save_service->local_param_copy.format)||
      // check F7 color mode change
      ((save_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)&&
       (save_service->current_buffer->format7_color_mode!=save_service->local_param_copy.format7_color_mode)
       ) ||
      // check bayer and stereo decoding
      (save_service->current_buffer->stereo_decoding!=save_service->local_param_copy.stereo_decoding)||
      (save_service->current_buffer->bayer!=save_service->local_param_copy.bayer)
      ) {
    if (save_service->current_buffer->width*save_service->current_buffer->height!=
	save_service->local_param_copy.width*save_service->local_param_copy.height) {
      buffer_size_change=1;
    }
    else {
      buffer_size_change=0;
    }
    
    // copy all new parameters:
    save_service->local_param_copy.width=save_service->current_buffer->width;
    save_service->local_param_copy.height=save_service->current_buffer->height;
    save_service->local_param_copy.bytes_per_frame=save_service->current_buffer->bytes_per_frame;
    save_service->local_param_copy.mode=save_service->current_buffer->mode;
    save_service->local_param_copy.format=save_service->current_buffer->format;
    save_service->local_param_copy.format7_color_mode=save_service->current_buffer->format7_color_mode;
    save_service->local_param_copy.stereo_decoding=save_service->current_buffer->stereo_decoding;
    save_service->local_param_copy.bayer=save_service->current_buffer->bayer;
    save_service->local_param_copy.buffer_image_bytes=save_service->current_buffer->buffer_image_bytes;
    
    // DO SOMETHING
    if (buffer_size_change!=0) {
      
      if (info->buffer!=NULL) {
	free(info->buffer);
	info->buffer=NULL;
      }
      info->buffer=(unsigned char*)malloc(save_service->current_buffer->width*save_service->current_buffer->height*3
					       *sizeof(unsigned char));
      if (info->buffer==NULL)
	fprintf(stderr,"Can't allocate buffer! Aiiieee!\n");
    }
  }
  
}
