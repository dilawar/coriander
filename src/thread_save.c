/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *
 * PVN saving capability by Jacob (Jack) Gryn and Konstantinos G. Derpanis
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
    strcpy(info->filename_base, cam->prefs.save_filename_base);
    strcpy(info->filename_ext, cam->prefs.save_filename_ext);

    /*
    // if we use conversion, look for the extension.
    if (cam->prefs.save_convert==SAVE_CONVERT_ON) {    
      if (tmp==NULL) {
	MainError("You should supply an extension");
	pthread_mutex_unlock(&save_service->mutex_data);
	FreeChain(save_service);
	return(-1);
      }
    }
    */

    info->period=cam->prefs.save_period;
    CommonChainSetup(cam, save_service,SERVICE_SAVE);
    

    ////////////////////////// REMOVE THESE COPIES ///////////////////////
    /// we freeze the gui controls to avoid any changes instead //////////
    /// cleanup to be performed on every service /////////////////////////
    //////// THIS SHOULD BE DONE FOR EVERY SERVICE ///////////////////////

    info->buffer=NULL;
    //info->mode=cam->prefs.save_mode;
    info->format=cam->prefs.save_format;
    info->append=cam->prefs.save_append;
    info->save_to_dir=cam->prefs.save_to_dir;
    // if format extension is ".raw", we dump raw data on the file and perform no conversion
    //info->rawdump=cam->prefs.save_convert;
    info->use_ram_buffer=cam->prefs.use_ram_buffer;
    info->ram_buffer_size=cam->prefs.ram_buffer_size*1024*1024; // ram buffer size in MB

    info->bigbuffer=NULL;

    if ((info->use_ram_buffer==TRUE)&&
	((info->format==SAVE_FORMAT_RAW_VIDEO)||
	 (info->format==SAVE_FORMAT_MPEG)||
	 (info->format==SAVE_FORMAT_PVN))) {
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

double
framerateAsDouble(int framerate_enum)
{
  switch(framerate_enum)  {
  case FRAMERATE_1_875:
    return(1.875);
    break;
  case FRAMERATE_3_75:
    return(3.750);
    break;
  case FRAMERATE_7_5:
    return(7.500);
    break;
  case FRAMERATE_15:
    return(15.000);
    break;
  case FRAMERATE_30:
    return(30.000);
    break;
  case FRAMERATE_60:
    return(60.000);
    break;
  case FRAMERATE_120:
    return(120.000);
    break;
  case FRAMERATE_240:
    return(240.000);
    break;
  default:
    return(0);
    break;
  }
  return(0);
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


// return Bytes in each pixel (3*bytes per channel if colour)
float
getAvgBytesPerPixel(int buffer_color_mode)
{
  switch(buffer_color_mode) {
  case COLOR_FORMAT7_MONO8:
  case COLOR_FORMAT7_RAW8:
    return(1.0);
    break;
  case COLOR_FORMAT7_YUV411:
    return(1.5);
    break;
  case COLOR_FORMAT7_MONO16:
  case COLOR_FORMAT7_RAW16:
  case COLOR_FORMAT7_MONO16S:
  case COLOR_FORMAT7_YUV422:
    return(2.0);
    break;
  case COLOR_FORMAT7_YUV444:
  case COLOR_FORMAT7_RGB8:
    return(3.0);
    break;
  case COLOR_FORMAT7_RGB16:
    return(6.0);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(0.0);
    break;
  }
  return(0.0);
}


unsigned int
getDepth(unsigned long bufsize, int mode, unsigned int height, unsigned int width)
{
  int bytes_per_pixel=getAvgBytesPerPixel(mode);
  return((unsigned int)bufsize/(bytes_per_pixel*height*width));
}

void
convert_for_pvn(unsigned char *buffer, unsigned int width, unsigned int height,
		unsigned int page, int buffer_color_mode, unsigned char *dest)
{
  unsigned char *buf_loc=buffer+(int)(page*getAvgBytesPerPixel(buffer_color_mode)*width*height);

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
  char magic[6];

  if(isColor(mode)==FALSE) {//greyscale
    if(mode == COLOR_FORMAT7_MONO16S)
      strcpy(magic,"PV5b");
    else
      strcpy(magic,"PV5a");
  }
  else // colour
    strcpy(magic,"PV6a");

  fprintf(fd,"%s\n%d %d %d\n%d %f\n", magic, width, height, depth, bpp, framerate);
}

static gint
CreateSettingsFile(char *destdir)
{
  char *fname = NULL;
  FILE *fd = NULL;
  int i;
  extern char* fps_label_list[NUM_FRAMERATES];
  extern char* feature_name_list[NUM_FEATURES];
  extern Prefs_t preferences;
  
  fname = (char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(fname,"%s/camera_setup.txt",destdir);
  fd = fopen(fname,"w");
  if (fd==NULL) {
    MainError("Cannot open settings file - write permission error");
    return(0);
  }
  
  fprintf(fd,"fps=%s\n", fps_label_list[camera->misc_info.framerate-FRAMERATE_MIN]);
  fprintf(fd,"sync_control=%d\n",preferences.sync_control);
  
  for(i=FEATURE_MIN; i<=FEATURE_MAX; ++i) {
    if (camera->feature_set.feature[i-FEATURE_MIN].available)
      fprintf(fd,"%s=%d\n", feature_name_list[i-FEATURE_MIN],
	      camera->feature_set.feature[i-FEATURE_MIN].value);
  }

  fclose(fd);
  return(1);
}


void
GetSaveFD(chain_t *save_service, FILE **fd, char *filename_out)
{
  savethread_info_t *info=save_service->data;
  // NOTE: the jpeg format is now joined with other imlib formats, but will have to be handled separately by ffmpeg (patch pending)

  // get filename
  switch (info->format) {
  case SAVE_FORMAT_PNG:
  case SAVE_FORMAT_JPEG:
  case SAVE_FORMAT_TIFF:
  case SAVE_FORMAT_PPMPGM:
  case SAVE_FORMAT_XPM:
  case SAVE_FORMAT_EIM:
  case SAVE_FORMAT_RAW:
    // first handle the case of save-to-dir
    if (info->save_to_dir==0) {
      switch (info->append) {
      case SAVE_APPEND_NONE:
	sprintf(filename_out, "%s%s", info->filename_base,info->filename_ext);
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s-%s%s", info->filename_base, save_service->current_buffer->captime_string, info->filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s-%10.10lli%s", info->filename_base, save_service->processed_frames, info->filename_ext);
	break;
      }
    }
    else { // we save to a directory...
      // 1. create the directory and write a setup file if it's the first frame
      if (save_service->processed_frames==0) {
	// note that we append a time tag to allow safe re-launch of the thread (it will prevent overwriting
	// previous results)
	sprintf(info->destdir,"%s-%s",info->filename_base,save_service->current_buffer->captime_string);

	// Optional: get rid of "-mmm" ms 
	//if (strlen(destdir) > 4)
	//destdir[strlen(destdir)-4]=0;
	
	if (mkdir(info->destdir,0755)) {
	  MainError("Could not create directory");
	  info->cancel_req = 1; // not threadsafe, but should be ok
	  return;
	}
	// Create a file with camera settings
	CreateSettingsFile(info->destdir);
      }

      // 2. build the filename
      switch (info->append) {
      case SAVE_APPEND_NONE: 
	fprintf(stderr,"time or number should have been selected\n");
	info->cancel_req = 1;
	return;
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s/%s%s", info->destdir, save_service->current_buffer->captime_string, info->filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s/%10.10lli%s", info->destdir, save_service->processed_frames, info->filename_ext);
	break;
      }
      // 3. done!
    }
    break;
  case SAVE_FORMAT_RAW_VIDEO:
  case SAVE_FORMAT_MPEG:
  case SAVE_FORMAT_PVN:
    switch (info->append) {
    case SAVE_APPEND_NONE:
      sprintf(filename_out, "%s%s", info->filename_base,info->filename_ext);
      break;
    case SAVE_APPEND_DATE_TIME:
      sprintf(filename_out, "%s-%s%s", info->filename_base, save_service->current_buffer->captime_string, info->filename_ext);
      break;
    case SAVE_APPEND_NUMBER:
      sprintf(filename_out,"%s-%10.10lli%s", info->filename_base, save_service->processed_frames, info->filename_ext);
      break;
    }
    break;
  }

  // open FD if it's the first frame of a video
  // OR it's in picture saving mode AND it's not using imlib (which creates the fd from the filename itself)
  switch (info->format) {
  case SAVE_FORMAT_RAW_VIDEO:
  case SAVE_FORMAT_MPEG:
  case SAVE_FORMAT_PVN:
    if (save_service->processed_frames==0) {
      *fd=fopen(filename_out,"w");
      if (*fd==NULL) {
	MainError("Can't create sequence file for saving");
	info->cancel_req = 1;
	return;
      }
    }
    break;
  case SAVE_FORMAT_RAW:
    *fd=fopen(filename_out,"w");
    if (*fd==NULL) {
      MainError("Can't create sequence file for saving");
      info->cancel_req = 1;
      return;
    }
  case SAVE_FORMAT_PNG:
  case SAVE_FORMAT_JPEG:
  case SAVE_FORMAT_TIFF:
  case SAVE_FORMAT_PPMPGM:
  case SAVE_FORMAT_XPM:
  case SAVE_FORMAT_EIM:
    // do nothing
    break;
  }
}


void
InitVideoFile(chain_t *save_service, FILE *fd)
{
  savethread_info_t *info;

  info=(savethread_info_t*)save_service->data;

  // (JG) if extension is PVN, write PVN header here
  if ((info->format==SAVE_FORMAT_PVN) && (info->use_ram_buffer==FALSE)) {
    fprintf(stderr,"pvn header write\n");
    writePVNHeader(fd, save_service->current_buffer->buffer_color_mode,
		   save_service->current_buffer->height,
		   save_service->current_buffer->width,
		   0, getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8,
		   framerateAsDouble(camera->misc_info.framerate));
  }
  
  // other inits for other video formats come here...
  // ...
  
}

void
FillRamBuffer(chain_t *save_service)
{
  savethread_info_t *info;

  info=(savethread_info_t*)save_service->data;

  if ((info->use_ram_buffer==TRUE)&&
      ((info->format==SAVE_FORMAT_RAW_VIDEO)||
       (info->format==SAVE_FORMAT_MPEG)||
       (info->format==SAVE_FORMAT_PVN))) {
    if (info->ram_buffer_size-info->bigbuffer_position>=save_service->current_buffer->buffer_image_bytes) {
      memcpy(&info->bigbuffer[info->bigbuffer_position], save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes);
      info->bigbuffer_position+=save_service->current_buffer->buffer_image_bytes;
    }
    else { // buffer is full, exit thread
      info->cancel_req=1;
    }
  }
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

	    // get file descriptor
	    GetSaveFD(save_service, &fd, filename_out);

	    // write initial data for video (header,...)
	    if ((save_service->processed_frames==0)&&
		((info->format==SAVE_FORMAT_RAW_VIDEO)||
		 (info->format==SAVE_FORMAT_MPEG)||
		 (info->format==SAVE_FORMAT_PVN))) {
	      InitVideoFile(save_service, fd);
	    }

	    // rambuffer operation
	    if ((info->use_ram_buffer==TRUE)&&
		((info->format==SAVE_FORMAT_RAW_VIDEO)||
		 (info->format==SAVE_FORMAT_MPEG)||
		 (info->format==SAVE_FORMAT_PVN))) {
	      FillRamBuffer(save_service);
	    }
	    else { // normal operation (no RAM buffer)
	      switch (info->format) {
	      case SAVE_FORMAT_RAW:
		fwrite(save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes, 1, fd);
		fclose(fd);
		break;
	      case SAVE_FORMAT_RAW_VIDEO:
		fwrite(save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes, 1, fd);
		break;
	      case SAVE_FORMAT_PVN:
		if (needsConversionForPVN(save_service->current_buffer->buffer_color_mode)>0) {
		  // we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
		  if (dest==NULL)
		    dest = (unsigned char*)malloc(3*save_service->current_buffer->width*save_service->current_buffer->height*sizeof(unsigned char));
		  convert_for_pvn(save_service->current_buffer->image, save_service->current_buffer->width,
				  save_service->current_buffer->height, 0, save_service->current_buffer->buffer_color_mode, dest);
		  fwrite(dest, 3*save_service->current_buffer->width*save_service->current_buffer->height, 1, fd);
		}
		else {
		  // no conversion, we can dump the data
		  fwrite(save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes, 1, fd);
		}
		break;
	      case SAVE_FORMAT_MPEG:
		MainError("MPEG not supported yet");
		info->cancel_req=1;
		break;
	      case SAVE_FORMAT_PNG:
	      case SAVE_FORMAT_JPEG: // <<<<<<<<<<<<< to be updated by jpeg/ffmpeg patch
	      case SAVE_FORMAT_TIFF:
	      case SAVE_FORMAT_PPMPGM:
	      case SAVE_FORMAT_XPM:
	      case SAVE_FORMAT_EIM:
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
		break;

	      } // end save format switch
	    } // end ram buffer if
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

  // we now have to close the video file properly and handle ram buffer operation

  if ((info->use_ram_buffer==TRUE)&&
      ((info->format==SAVE_FORMAT_RAW_VIDEO)||
       (info->format==SAVE_FORMAT_MPEG)||
       (info->format==SAVE_FORMAT_PVN))) {
    fwrite(info->bigbuffer, 1, info->bigbuffer_position, fd);
  }

  if ((info->use_ram_buffer==TRUE)&&(info->format==SAVE_FORMAT_PVN)) {
    writePVNHeader(fd, save_service->current_buffer->buffer_color_mode,
		   save_service->current_buffer->height,
		   save_service->current_buffer->width,
		   getDepth(info->bigbuffer_position, save_service->current_buffer->buffer_color_mode, 
			    save_service->current_buffer->height, save_service->current_buffer->width),
		   getConvertedBytesPerChannel(save_service->current_buffer->buffer_color_mode)*8,
		            framerateAsDouble(camera->misc_info.framerate));

    if(needsConversionForPVN(save_service->current_buffer->buffer_color_mode)==FALSE) {
      fwrite(info->bigbuffer, 1, info->bigbuffer_position, fd);
    }
    else {
      // we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
      if (dest==NULL)
	dest = (unsigned char*)malloc(3*save_service->current_buffer->width*save_service->current_buffer->height*sizeof(unsigned char));
      
      for (i = 0; i < getDepth(info->bigbuffer_position, save_service->current_buffer->buffer_color_mode,
			       save_service->current_buffer->height, save_service->current_buffer->width); i++) {
        convert_for_pvn(info->bigbuffer, save_service->current_buffer->width,
			save_service->current_buffer->height, i, save_service->current_buffer->buffer_color_mode, dest);
        fwrite(dest, 1, 3*save_service->current_buffer->width*save_service->current_buffer->height, fd);
      }
    }
  }
  
  if ((fd!=NULL) &&
      ((info->format==SAVE_FORMAT_RAW_VIDEO)||
       (info->format==SAVE_FORMAT_MPEG)||
       (info->format==SAVE_FORMAT_PVN))) {
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
