/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Dan Dennedy <dan@dennedy.org>
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

#include "thread_v4l.h"

extern PrefsInfo preferences;
extern GtkWidget *main_window;
extern CtxtInfo ctxt;
extern camera_t* camera;

gint
V4lStartThread(camera_t* cam)
{
  chain_t* v4l_service=NULL;
  v4lthread_info_t *info=NULL;
  char stemp[STRING_SIZE];

  v4l_service=GetService(camera, SERVICE_V4L);

  if (v4l_service==NULL) { // if no V4L service running...
    //fprintf(stderr,"No V4L service found, inserting new one\n");
    v4l_service=(chain_t*)malloc(sizeof(chain_t));
    v4l_service->current_buffer=NULL;
    v4l_service->next_buffer=NULL;
    v4l_service->data=(void*)malloc(sizeof(v4lthread_info_t));
    info=(v4lthread_info_t*)v4l_service->data;
    pthread_mutex_init(&v4l_service->mutex_data, NULL);
    pthread_mutex_init(&v4l_service->mutex_struct, NULL);
    pthread_mutex_init(&info->mutex_cancel_v4l, NULL);
    
    /* if you want a clean-interrupt thread:*/
    pthread_mutex_lock(&info->mutex_cancel_v4l);
    info->cancel_v4l_req=0;
    pthread_mutex_unlock(&info->mutex_cancel_v4l);
    
    /* setup v4l_thread: handles, ...*/
    pthread_mutex_lock(&v4l_service->mutex_data);
    
    info->period=preferences.v4l_period;
    CommonChainSetup(cam, v4l_service,SERVICE_V4L);
    
    info->v4l_buffer=NULL;
    //info->counter=0;

    // COPY PREFERENCES HERE
    strcpy(info->v4l_dev_name,preferences.v4l_dev_name);
    //info->v4l_scratch=preferences.v4l_scratch;
    //info->rawdump=preferences.v4l_convert;

    // open V4L device
    info->v4l_dev=-1;
    info->v4l_dev = open(info->v4l_dev_name, O_RDWR);
    if (info->v4l_dev < 0) {
      sprintf(stemp,"Failed to open V4L device %s",info->v4l_dev_name);
      MainError(stemp);
      FreeChain(v4l_service);
      return(-1);
    }

    pthread_mutex_unlock(&v4l_service->mutex_data);
    
    /* Insert chain and start service*/
    pthread_mutex_lock(&v4l_service->mutex_struct);
    InsertChain(cam, v4l_service);
    pthread_mutex_unlock(&v4l_service->mutex_struct);
    
    pthread_mutex_lock(&v4l_service->mutex_data);
    pthread_mutex_lock(&v4l_service->mutex_struct);
    if (pthread_create(&v4l_service->thread, NULL, V4lThread,(void*) v4l_service)) {
      /* error starting thread. You should cleanup here
	 (free, unset global vars,...):*/
      
      /* Mendatory cleanups:*/
      RemoveChain(cam, v4l_service);
      pthread_mutex_unlock(&v4l_service->mutex_struct);
      pthread_mutex_unlock(&v4l_service->mutex_data);
      free(info->v4l_buffer);
      FreeChain(v4l_service);
      return(-1);
    }
    info->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)V4lShowFPS, (gpointer*) v4l_service);
    pthread_mutex_unlock(&v4l_service->mutex_struct);
    pthread_mutex_unlock(&v4l_service->mutex_data);
    
  }
  //fprintf(stderr," V4L service started\n");
  
  return (1);
}


void*
V4lCleanupThread(void* arg) 
{
  chain_t* v4l_service;
  v4lthread_info_t *info;

  v4l_service=(chain_t*)arg;
  info=(v4lthread_info_t*)v4l_service->data;
  /* Specific cleanups: */

  /* Mendatory cleanups: */
  pthread_mutex_unlock(&v4l_service->mutex_data);

  return(NULL);
}

int
V4lShowFPS(gpointer *data)
{
  chain_t* v4l_service;
  v4lthread_info_t *info;
  char tmp_string[20];
  float tmp, fps;

  v4l_service=(chain_t*)data;
  info=(v4lthread_info_t*)v4l_service->data;

  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=fabs(0.0);
  else
    fps=fabs((float)info->frames/tmp);
  
  sprintf(tmp_string," %.2f",fps);

  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_v4l"),
		       ctxt.fps_v4l_ctxt, ctxt.fps_v4l_id);
  ctxt.fps_v4l_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_v4l"),
					 ctxt.fps_v4l_ctxt, tmp_string);
  
  pthread_mutex_lock(&v4l_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&v4l_service->mutex_data);

  return 1;
}


void*
V4lThread(void* arg)
{
  chain_t* v4l_service=NULL;
  v4lthread_info_t *info=NULL;
  long int skip_counter;
  
  v4l_service=(chain_t*)arg;
  pthread_mutex_lock(&v4l_service->mutex_data);
  info=(v4lthread_info_t*)v4l_service->data;
  skip_counter=0;

  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&v4l_service->mutex_data);
  
  // time inits:
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

  while (1) { 
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel_v4l);
    if (info->cancel_v4l_req>0) {
      pthread_mutex_unlock(&info->mutex_cancel_v4l);
      return ((void*)1);
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel_v4l);
      pthread_mutex_lock(&v4l_service->mutex_data);
      if(RollBuffers(v4l_service)) { // have buffers been rolled?
	// check params
	V4lThreadCheckParams(v4l_service);
	
	convert_to_rgb(v4l_service->current_buffer, info->v4l_buffer);

	swap_rb(info->v4l_buffer, v4l_service->current_buffer->width*v4l_service->current_buffer->height*3);

	if (v4l_service->current_buffer->width!=-1) {
	  if (skip_counter==(info->period-1)) {
	    skip_counter=0;
	    write(info->v4l_dev,info->v4l_buffer,v4l_service->current_buffer->width*v4l_service->current_buffer->height*3);
	    info->frames++;
	  }
	  else
	    skip_counter++;
	  
	  // FPS display
	  info->current_time=times(&info->tms_buf);
	}
	pthread_mutex_unlock(&v4l_service->mutex_data);
      }
      else {
	pthread_mutex_unlock(&v4l_service->mutex_data);
	usleep(THREAD_LOOP_SLEEP_TIME_US);
      }
    }
  }
}


gint
V4lStopThread(camera_t* cam)
{
  v4lthread_info_t *info;
  chain_t *v4l_service;
  v4l_service=GetService(cam,SERVICE_V4L);

  if (v4l_service!=NULL) { // if V4L service running...
    //fprintf(stderr,"V4L service found, stopping\n");
    info=(v4lthread_info_t*)v4l_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel_v4l);
    info->cancel_v4l_req=1;
    pthread_mutex_unlock(&info->mutex_cancel_v4l);
    
    /* common handlers...*/
    pthread_join(v4l_service->thread, NULL);
    
    pthread_mutex_lock(&v4l_service->mutex_data);
    pthread_mutex_lock(&v4l_service->mutex_struct);
    
    gtk_timeout_remove(info->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_v4l"), ctxt.fps_v4l_ctxt, ctxt.fps_v4l_id);
    ctxt.fps_v4l_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_v4l"), ctxt.fps_v4l_ctxt, "");
    
    RemoveChain(cam,v4l_service);
    
    /* Do custom cleanups here...*/
    if (info->v4l_buffer!=NULL) {
      free(info->v4l_buffer);
      info->v4l_buffer=NULL;
    }
    close(info->v4l_dev);

    /* Mendatory cleanups: */
    pthread_mutex_unlock(&v4l_service->mutex_struct);
    pthread_mutex_unlock(&v4l_service->mutex_data);
    FreeChain(v4l_service);
    
    //fprintf(stderr," V4L service stopped\n");
  }
  
  return (1);
}

void
V4lThreadCheckParams(chain_t *v4l_service)
{

  v4lthread_info_t *info;
  int buffer_size_change=0;
  info=(v4lthread_info_t*)v4l_service->data;

  // copy harmless parameters anyway:
  v4l_service->local_param_copy.bpp=v4l_service->current_buffer->bpp;
  v4l_service->local_param_copy.bayer_pattern=v4l_service->current_buffer->bayer_pattern;

  // if some parameters changed, we need to re-allocate the local buffers and restart the v4l
  if ((v4l_service->current_buffer->width!=v4l_service->local_param_copy.width)||
      (v4l_service->current_buffer->height!=v4l_service->local_param_copy.height)||
      (v4l_service->current_buffer->bytes_per_frame!=v4l_service->local_param_copy.bytes_per_frame)||
      (v4l_service->current_buffer->mode!=v4l_service->local_param_copy.mode)||
      (v4l_service->current_buffer->format!=v4l_service->local_param_copy.format)||
      // check F7 color mode change
      ((v4l_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)&&
       (v4l_service->current_buffer->format7_color_mode!=v4l_service->local_param_copy.format7_color_mode)
       ) ||
      // check bayer and stereo decoding
      (v4l_service->current_buffer->stereo_decoding!=v4l_service->local_param_copy.stereo_decoding)||
      (v4l_service->current_buffer->bayer!=v4l_service->local_param_copy.bayer)
      ) {
    if (v4l_service->current_buffer->width*v4l_service->current_buffer->height!=
	v4l_service->local_param_copy.width*v4l_service->local_param_copy.height) {
      buffer_size_change=1;
    }
    else {
      buffer_size_change=0;
    }
    
    // copy all new parameters:
    v4l_service->local_param_copy.width=v4l_service->current_buffer->width;
    v4l_service->local_param_copy.height=v4l_service->current_buffer->height;
    v4l_service->local_param_copy.bytes_per_frame=v4l_service->current_buffer->bytes_per_frame;
    v4l_service->local_param_copy.mode=v4l_service->current_buffer->mode;
    v4l_service->local_param_copy.format=v4l_service->current_buffer->format;
    v4l_service->local_param_copy.format7_color_mode=v4l_service->current_buffer->format7_color_mode;
    v4l_service->local_param_copy.stereo_decoding=v4l_service->current_buffer->stereo_decoding;
    v4l_service->local_param_copy.bayer=v4l_service->current_buffer->bayer;
    
    // DO SOMETHING
    if (buffer_size_change!=0) {
      
      // clear buffer
      if (info->v4l_buffer!=NULL) {
	free(info->v4l_buffer);
	info->v4l_buffer=NULL;
      }

      // create buffer
      info->v4l_buffer=(unsigned char*)malloc(v4l_service->current_buffer->width*v4l_service->current_buffer->height
					      *3*sizeof(unsigned char));
      if (info->v4l_buffer==NULL) {
	fprintf(stderr,"Can't allocate buffer! Aiiieee!\n");
      }
      
      // STOP PIPE MIGHT BE NECESSARY HERE

      // "start pipe"      
      if (ioctl (info->v4l_dev, VIDIOCGCAP, &info->vid_caps) == -1) {
	perror ("ioctl (VIDIOCGCAP)");
      }
      if (ioctl (info->v4l_dev, VIDIOCGPICT, &info->vid_pic)== -1) {
	perror ("ioctl VIDIOCGPICT");
      }
      info->vid_pic.palette = VIDEO_PALETTE_RGB24;
      if (ioctl (info->v4l_dev, VIDIOCSPICT, &info->vid_pic)== -1) {
	perror ("ioctl VIDIOCSPICT");
      }
      if (ioctl (info->v4l_dev, VIDIOCGWIN, &info->vid_win)== -1) {
	perror ("ioctl VIDIOCGWIN");
      }
      info->vid_win.width=v4l_service->current_buffer->width;
      info->vid_win.height=v4l_service->current_buffer->height;
      if (ioctl (info->v4l_dev, VIDIOCSWIN, &info->vid_win)== -1) {
	perror ("ioctl VIDIOCSWIN");
      }
    }
  }
}

void
swap_rb(unsigned char *image, int i) {

  unsigned char t;
  i--;
  //fprintf(stderr,"first image index: %d\n",i);

  while (i>0) {
    t=image[i];
    image[i]=image[i-2];
    image[i-2]=t;
    i-=3;
  }

}