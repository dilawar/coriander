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

#include "thread_save.h" 

extern PrefsInfo preferences;
extern GtkWidget *main_window;
extern CtxtInfo ctxt;
extern camera_t* camera;

gint
SaveStartThread(camera_t* cam)
{
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  gchar *tmp;

  save_service=GetService(camera, SERVICE_SAVE);

  if (save_service==NULL) { // if no SAVE service running...
    //fprintf(stderr,"No SAVE service found, inserting new one\n");
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
    strcpy(info->filename, preferences.save_filename);
    tmp = strrchr(info->filename, '.');
    
    if (tmp==NULL) {
      MainError("You should supply an extension");
      pthread_mutex_unlock(&save_service->mutex_data);
      FreeChain(save_service);
      return(-1);
    }
    
    tmp[0] = '\0';// cut filename before point
    strcpy(info->filename_ext, strrchr(preferences.save_filename, '.'));
    
    info->period=preferences.save_period;
    CommonChainSetup(cam, save_service,SERVICE_SAVE);
    
    info->buffer=NULL;
    info->counter=0;
    info->scratch=preferences.save_scratch;
    info->datenum=preferences.save_datenum;
    // if format extension is ".raw", we dump raw data on the file and perform no conversion
    info->rawdump=preferences.save_convert;
    info->use_ram_buffer=preferences.use_ram_buffer;
    info->ram_buffer_size=preferences.ram_buffer_size*1024*1024; // ram buffer size in MB

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

    //pthread_mutex_unlock(&save_service->mutex_data);
    
    /* Insert chain and start service*/
    pthread_mutex_lock(&save_service->mutex_struct);
    InsertChain(cam, save_service);
    //pthread_mutex_unlock(&save_service->mutex_struct);
    
    //pthread_mutex_lock(&save_service->mutex_data);
    //pthread_mutex_lock(&save_service->mutex_struct);
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
    info->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)SaveShowFPS, (gpointer*) save_service);
    pthread_mutex_unlock(&save_service->mutex_struct);
    pthread_mutex_unlock(&save_service->mutex_data);
    
  }
  //fprintf(stderr," SAVE service started\n");
  
  return (1);
}

int
SaveShowFPS(gpointer *data)
{
  chain_t* save_service;
  savethread_info_t *info;
  char tmp_string[20];
  float tmp, fps;

  save_service=(chain_t*)data;
  info=(savethread_info_t*)save_service->data;

  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=fabs(0.0);
  else
    fps=fabs((float)info->frames/tmp);
  
  sprintf(tmp_string," %.2f",fps);

  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_save"),
		       ctxt.fps_save_ctxt, ctxt.fps_save_id);
  ctxt.fps_save_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_save"),
					 ctxt.fps_save_ctxt, tmp_string);
  
  pthread_mutex_lock(&save_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&save_service->mutex_data);

  return 1;
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

  /* Mendatory cleanups: */
  //pthread_mutex_unlock(&save_service->mutex_data);

  //fprintf(stderr,"cleanup thread!<n");

  return(NULL);
}

void*
SaveThread(void* arg)
{
  static gchar filename_out[STRING_SIZE];
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  GdkImlibImage *im=NULL;
  long int skip_counter;
  FILE *fd=NULL;

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
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

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
	    case SAVE_SCRATCH_OVERWRITE:
	      sprintf(filename_out, "%s%s", info->filename,info->filename_ext);
	      fd=fopen(filename_out,"w");
	      if (fd==NULL)
		MainError("Can't create/open image file for saving");
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
	      fd=fopen(filename_out,"w");
	      if (fd==NULL)
		MainError("Can't create/open image file for saving");
	      break;
	    default:
	      break;
	    }
	    
	    // rambuffer operation
	    if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)) {
	      if (info->ram_buffer_size-info->bigbuffer_position>=save_service->current_buffer->buffer_image_bytes) {
		memcpy(&info->bigbuffer[info->bigbuffer_position], save_service->current_buffer->image, save_service->current_buffer->buffer_image_bytes);
		info->bigbuffer_position+=save_service->current_buffer->buffer_image_bytes;
		//fprintf(stderr,"remains: %ld bytes\n",info->ram_buffer_size-info->bigbuffer_position);
	      }
	      else {
		// buffer is full, exit thread
		info->cancel_req=1;
	      }
	    }
	    // normal operation
	    else {
	      if (info->rawdump>0) {
		if (info->scratch==SAVE_SCRATCH_VIDEO) {
		  fwrite(save_service->current_buffer->image, 1, save_service->current_buffer->buffer_image_bytes, fd);
		}
		else {
		  Dump2File(filename_out,save_service);
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
	    info->frames++;

	  }
	  else
	    skip_counter++;
	  
	  // FPS display
	  info->current_time=times(&info->tms_buf);
	}
	pthread_mutex_unlock(&save_service->mutex_data);
      }
      else {
	pthread_mutex_unlock(&save_service->mutex_data);
	usleep(THREAD_LOOP_SLEEP_TIME_US);
      }
    }
  }
  if ((info->use_ram_buffer==TRUE)&&(info->scratch==SAVE_SCRATCH_VIDEO)) {
    fwrite(info->bigbuffer, 1, info->bigbuffer_position, fd);
  }

  if ((info->scratch==SAVE_SCRATCH_VIDEO)&&(fd!=NULL)) {
    fclose(fd);
  }

  if (info->bigbuffer!=NULL) {
    free(info->bigbuffer);
  }
  pthread_mutex_unlock(&info->mutex_cancel);
  return ((void*)1);
}


gint
SaveStopThread(camera_t* cam)
{
  savethread_info_t *info;
  chain_t *save_service;
  save_service=GetService(cam,SERVICE_SAVE);

  if (save_service!=NULL) { // if SAVE service running...
    //fprintf(stderr,"SAVE service found, stopping\n");
    info=(savethread_info_t*)save_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* common handlers...*/
    pthread_join(save_service->thread, NULL);
    
    pthread_mutex_lock(&save_service->mutex_data);
    pthread_mutex_lock(&save_service->mutex_struct);
    
    gtk_timeout_remove(info->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_save"), ctxt.fps_save_ctxt, ctxt.fps_save_id);
    ctxt.fps_save_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_save"), ctxt.fps_save_ctxt, "");
    
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
    
    //fprintf(stderr," SAVE service stopped\n");
  }
  
  return (1);
}

void
Dump2File(char *name, chain_t *service)
{
  FILE *fd;
  fd=fopen(name,"w");
  if (fd==NULL)
    MainError("Can't open file for saving");
  else {
    fwrite(service->current_buffer->image, 1, service->current_buffer->buffer_image_bytes, fd);
    fclose(fd);
  }
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
