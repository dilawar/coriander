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

#include "thread_display.h"

extern PrefsInfo preferences;
extern GtkWidget *main_window;
extern CtxtInfo ctxt;
extern camera_t* camera;

#ifdef HAVE_SDLLIB
extern watchthread_info_t watchthread_info;
#endif

gint
DisplayStartThread(camera_t* cam)
{
  chain_t *display_service=NULL;
  displaythread_info_t *info=NULL;

  display_service=GetService(camera,SERVICE_DISPLAY);

  if (display_service==NULL) {// if no display service running...
    //fprintf(stderr,"No DISPLAY service found, inserting new one\n");
    display_service=(chain_t*)malloc(sizeof(chain_t));
    display_service->current_buffer=NULL;
    display_service->next_buffer=NULL;
    display_service->data=(void*)malloc(sizeof(displaythread_info_t));
    info=(displaythread_info_t*)display_service->data;
    pthread_mutex_init(&display_service->mutex_struct, NULL);
    pthread_mutex_init(&display_service->mutex_data, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    pthread_mutex_lock(&display_service->mutex_data);
    info->period=preferences.display_period;
    CommonChainSetup(cam, display_service,SERVICE_DISPLAY);
    
    //pthread_mutex_unlock(&display_service->mutex_data);
    
    pthread_mutex_lock(&display_service->mutex_struct);
    InsertChain(cam, display_service);
    //pthread_mutex_unlock(&display_service->mutex_struct);
    
    //pthread_mutex_lock(&display_service->mutex_data);
    //pthread_mutex_lock(&display_service->mutex_struct);
    if (pthread_create(&display_service->thread, NULL, DisplayThread, (void*)display_service)) {
      RemoveChain(cam, display_service);
      pthread_mutex_unlock(&display_service->mutex_struct);
      pthread_mutex_unlock(&display_service->mutex_data);
      FreeChain(display_service);
      return(-1);
    }
    info->timeout_func_id=gtk_timeout_add(1000, (GtkFunction)DisplayShowFPS, (gpointer*) display_service);
    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    //fprintf(stderr," DISPLAY service started\n");
    
  }
  return (1);
}


void*
DisplayCleanupThread(void* arg)
{
  chain_t* display_service;
  displaythread_info_t *info;

  display_service=(chain_t*)arg;
  info=(displaythread_info_t*)display_service->data;

  pthread_mutex_unlock(&display_service->mutex_data);

  return(NULL);
}

  
int
DisplayShowFPS(gpointer *data)
{
  chain_t* display_service;
  displaythread_info_t *info;
  float tmp, fps;
  char *tmp_string;

  tmp_string=(char*)malloc(20*sizeof(char));

  display_service=(chain_t*)data;
  info=(displaythread_info_t*)display_service->data;

  tmp=(float)(info->current_time-info->prev_time)/sysconf(_SC_CLK_TCK);
  if (tmp==0)
    fps=fabs(0.0);
  else
    fps=fabs((float)info->frames/tmp);

  sprintf(tmp_string," %.2f",fps);

  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_display"),
		       ctxt.fps_display_ctxt, ctxt.fps_display_id);
  ctxt.fps_display_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_display"),
					 ctxt.fps_display_ctxt, tmp_string);
  
  pthread_mutex_lock(&display_service->mutex_data);
  info->prev_time=info->current_time;
  info->frames=0;
  pthread_mutex_unlock(&display_service->mutex_data);

  free(tmp_string);

  return 1;
}


void*
DisplayThread(void* arg)
{
  chain_t* display_service=NULL;
  displaythread_info_t *info=NULL;
  long int skip_counter;

  // we should only use mutex_data in this function

  display_service=(chain_t*)arg;
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);
  skip_counter=0;

  // time inits:
  info->prev_time = times(&info->tms_buf);
  info->frames=0;

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel);
    
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&display_service->mutex_data);
      if(RollBuffers(display_service)) { // have buffers been rolled?
	// check params
	DisplayThreadCheckParams(display_service);
	if (display_service->current_buffer->width!=-1) {
	  if (skip_counter>=(info->period-1)) {
	    skip_counter=0;
#ifdef HAVE_SDLLIB
	    if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
	      convert_to_yuv_for_SDL(display_service->current_buffer, info->sdloverlay->pixels[0]);

	      SDLDisplayArea(display_service);

	      SDL_UnlockYUVOverlay(info->sdloverlay);
	      SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);

	      info->redraw_prev_time=times(&info->redraw_tms_buf);
	    }
#endif
	  info->frames++;
	  }
	  else { //
	    if (preferences.display_redraw==DISPLAY_REDRAW_ON) {
	      ConditionalTimeoutRedraw(display_service);
	    }
	    skip_counter++;
	  }
	  // FPS display:
	  info->current_time=times(&info->tms_buf);
	}
	
	pthread_mutex_unlock(&display_service->mutex_data);
      }
      else { //
	if (preferences.display_redraw==DISPLAY_REDRAW_ON) {
	  ConditionalTimeoutRedraw(display_service);
	}
	pthread_mutex_unlock(&display_service->mutex_data);
      }
    }
    usleep(0);
  }
  
  pthread_mutex_unlock(&info->mutex_cancel);
  return ((void*)1);
}

// the goal of the following function is to redraw the SDL display twice a second so that the image follows the screen
// during window movement or if another window comes momentarily on top of the display while no images are coming. 
void
ConditionalTimeoutRedraw(chain_t* service)
{
  displaythread_info_t *info=NULL;
  float interval;
  info=(displaythread_info_t*)service->data;

  if (service->current_buffer->width!=-1) {
    info->redraw_current_time=times(&info->redraw_tms_buf);
    interval=fabs((float)(info->redraw_current_time-info->redraw_prev_time)/sysconf(_SC_CLK_TCK));
    if (interval>(1.0/preferences.display_redraw_rate)) { // redraw a minimal 4 times per second
#ifdef HAVE_SDLLIB
      if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
	convert_to_yuv_for_SDL(service->current_buffer, info->sdloverlay->pixels[0]);
	SDLDisplayArea(service);
	SDL_UnlockYUVOverlay(info->sdloverlay);
	SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);
      }
#endif
      //fprintf(stderr,"Display redrawn with time interval of %.3f sec\n",interval);
      info->redraw_prev_time=times(&info->redraw_tms_buf);
    }
  }
}

gint
DisplayStopThread(camera_t* cam)
{
  displaythread_info_t *info;
  chain_t *display_service;
  display_service=GetService(cam,SERVICE_DISPLAY);
  
  if (display_service!=NULL) { // if display service running... 
    //fprintf(stderr,"DISPLAY service found, stopping\n");
    info=(displaythread_info_t*)display_service->data;
    
    // send request for cancellation:
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    // when cancellation occured, join:
    pthread_join(display_service->thread, NULL);
    
    pthread_mutex_lock(&display_service->mutex_data);
    pthread_mutex_lock(&display_service->mutex_struct);
    gtk_timeout_remove(info->timeout_func_id);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_display"),
			 ctxt.fps_display_ctxt, ctxt.fps_display_id);
    ctxt.fps_display_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_display"),
					   ctxt.fps_display_ctxt, "");
    
    RemoveChain(cam,display_service);
    SDLQuit(display_service);
    
    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    FreeChain(display_service);
    //fprintf(stderr," DISPLAY service stopped\n");
  }
  return (1);
}

#ifdef HAVE_SDLLIB

int
SDLInit(chain_t *display_service)
{
  displaythread_info_t *info;
  const SDL_VideoInfo *sdl_videoinfo;
  SDL_Rect** modes;
  info=(displaythread_info_t*)display_service->data;

  info->sdlbpp=16;
  info->sdlflags=SDL_ANYFORMAT | SDL_RESIZABLE;

  // Initialize the SDL library (video subsystem)
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
    MainError("Couldn't initialize SDL video subsystem");
    return(0);
  }
  
  sdl_videoinfo = SDL_GetVideoInfo();

  if (sdl_videoinfo->hw_available) {
    //fprintf(stderr,"Using HW surface\n");
    info->sdlflags|= SDL_HWSURFACE;
    info->sdlflags|= SDL_HWACCEL;
    info->sdlflags&= ~SDL_SWSURFACE;
  }
  else {
    //fprintf(stderr,"Using SW surface\n");
    info->sdlflags|= SDL_SWSURFACE;
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
  }

  // force the system to try hardware accel
  info->sdlflags|= SDL_HWSURFACE;
  info->sdlflags|= SDL_HWACCEL;

  modes=SDL_ListModes(NULL,info->sdlflags);
  if (modes!=(SDL_Rect**)-1) {
    // not all resolutions are OK for this video card. For safety we switch to software accel
    MainError("No SDL mode available with hardware accel. Trying without HWSURFACE");
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
    modes=SDL_ListModes(NULL,info->sdlflags);
    if (modes!=(SDL_Rect**)-1) {
      MainError("Still no modes available. Can't start SDL!");
      SDL_Quit();
      return(0);
    }
  }
    // Set requested video mode
  info->sdlbpp=SDL_VideoModeOK(display_service->current_buffer->width, display_service->current_buffer->height, 
			       info->sdlbpp, info->sdlflags);
  
  info->sdlvideo = SDL_SetVideoMode(display_service->current_buffer->width, display_service->current_buffer->height, 
				    info->sdlbpp, info->sdlflags);

  if (info->sdlvideo == NULL) {
    MainError(SDL_GetError());
    SDL_Quit();
    return(0);
  }

  if (SDL_SetColorKey( info->sdlvideo, SDL_SRCCOLORKEY, 0x0) < 0 ) {
    MainError(SDL_GetError());
  }
  
  // Show cursor
  SDL_ShowCursor(1);
  
  // set window title:
  SDL_WM_SetCaption(camera->name,"");

  // this line broke everything for unknown reasons so I just rmove it.
  //info->sdlvideo->format->BytesPerPixel=2;

  // Create YUV Overlay
  info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->width,
					  display_service->current_buffer->height,
					  SDL_YUY2_OVERLAY,info->sdlvideo);
  if (info->sdloverlay==NULL) {
    MainError(SDL_GetError());
    SDL_Quit();
    return(0);
  }

  info->sdlvideorect.x=0;
  info->sdlvideorect.y=0;
  info->sdlvideorect.w=display_service->current_buffer->width;
  info->sdlvideorect.h=display_service->current_buffer->height;

  watchthread_info.f7_step[0]=display_service->camera->format7_info.mode[display_service->current_buffer->mode-MODE_FORMAT7_MIN].step_x;
  watchthread_info.f7_step[1]=display_service->camera->format7_info.mode[display_service->current_buffer->mode-MODE_FORMAT7_MIN].step_y;
  watchthread_info.f7_step_pos[0]=display_service->camera->format7_info.mode[display_service->current_buffer->mode-MODE_FORMAT7_MIN].step_pos_x;
  watchthread_info.f7_step_pos[1]=display_service->camera->format7_info.mode[display_service->current_buffer->mode-MODE_FORMAT7_MIN].step_pos_y;
  watchthread_info.use_unit_pos=display_service->camera->format7_info.mode[display_service->current_buffer->mode-MODE_FORMAT7_MIN].use_unit_pos;

  SDLEventStartThread(display_service);

  return(1);
}

// we should optimize this for RGB too: RGB modes could use RGB-SDL instead of YUV overlay
void
convert_to_yuv_for_SDL(buffer_t *buffer, unsigned char *dest)
{
  switch(buffer->buffer_color_mode) {
  case COLOR_FORMAT7_MONO8:
    y2uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV411:
    uyyvyy2uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV422:
    yuyv2uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV444:
    uyv2uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_RGB8:
    rgb2uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_MONO16:
    y162uyvy(buffer->image,dest,buffer->width*buffer->height,buffer->bpp);
    break;
  case COLOR_FORMAT7_RGB16:
    rgb482uyvy(buffer->image,dest,buffer->width*buffer->height);
    break;
  }
}


void
SDLDisplayArea(chain_t *display_service)
{
  displaythread_info_t *info;
  unsigned char *pimage;
  int upper_left[2];
  int lower_right[2];
  int width;
  int tmp;
  register int i;
  register int j;
  info=(displaythread_info_t*)display_service->data;

  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (watchthread_info.draw==1) {
    upper_left[0]=watchthread_info.upper_left[0];
    upper_left[1]=watchthread_info.upper_left[1];
    lower_right[0]=watchthread_info.lower_right[0];
    lower_right[1]=watchthread_info.lower_right[1];
    pimage=info->sdloverlay->pixels[0];
    width=display_service->current_buffer->width;
    pthread_mutex_unlock(&watchthread_info.mutex_area);
    
    if (lower_right[0]<upper_left[0]) {
      tmp=lower_right[0];
      lower_right[0]=upper_left[0];
      upper_left[0]=tmp;
    }
    if (lower_right[1]<upper_left[1]) {
      tmp=lower_right[1];
      lower_right[1]=upper_left[1];
      upper_left[1]=tmp;
    }

    for (i=upper_left[1];i<=lower_right[1];i++)
      for (j=upper_left[0];j<=lower_right[0];j++)
	pimage[(i*width+j)*2]=(unsigned char)(255-pimage[(i*width+j)*2]);
    
  }
  else
    pthread_mutex_unlock(&watchthread_info.mutex_area);
  
}

void
SDLQuit(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

#ifdef HAVE_SDLLIB
  //fprintf(stderr,"about to shutdown SDL stuff\n");
  SDLEventStopThread(display_service);
  // if width==-1, SDL was never initialized so we do nothing
  if (display_service->current_buffer->width!=-1) {
    //fprintf(stderr,"event stopped\n");
    SDL_FreeYUVOverlay(info->sdloverlay);
    SDL_FreeSurface(info->sdlvideo);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }
#endif
}

void
DisplayThreadCheckParams(chain_t *display_service)
{

  displaythread_info_t *info;
  int first_time=0;
  int size_change;
  info=(displaythread_info_t*)display_service->data;
  
  // copy harmless parameters anyway:
  display_service->local_param_copy.bpp=display_service->current_buffer->bpp;
  display_service->local_param_copy.bayer_pattern=display_service->current_buffer->bayer_pattern;
  if (display_service->current_buffer->width==-1)
    fprintf(stderr,"Display size: %dx%d\n",display_service->current_buffer->width,display_service->current_buffer->height);

  // if some parameters changed, we need to re-allocate the local buffers and restart the display
  if ((display_service->current_buffer->width!=display_service->local_param_copy.width)||
      (display_service->current_buffer->height!=display_service->local_param_copy.height)||
      (display_service->current_buffer->bytes_per_frame!=display_service->local_param_copy.bytes_per_frame)||
      (display_service->current_buffer->mode!=display_service->local_param_copy.mode)||
      (display_service->current_buffer->format!=display_service->local_param_copy.format)||
      // check F7 color mode change
      ((display_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)&&
       (display_service->current_buffer->format7_color_mode!=display_service->local_param_copy.format7_color_mode)
       ) ||
      // check bayer and stereo decoding
      (display_service->current_buffer->stereo_decoding!=display_service->local_param_copy.stereo_decoding)||
      (display_service->current_buffer->bayer!=display_service->local_param_copy.bayer)
      ) {
    if ((display_service->local_param_copy.width==-1)&&(display_service->current_buffer->width!=-1)) {
      first_time=1;
    }
    else {
      first_time=0;
    }
    
    if ((display_service->current_buffer->width!=display_service->local_param_copy.width)||
	(display_service->current_buffer->height!=display_service->local_param_copy.height)) {
      size_change=1;
      //fprintf(stderr,"Display size change: %d x %d\n",display_service->current_buffer->width,display_service->current_buffer->height);
    }
    else {
      size_change=0;
    }
    
    // copy all new parameters:
    display_service->local_param_copy.width=display_service->current_buffer->width;
    display_service->local_param_copy.height=display_service->current_buffer->height;
    display_service->local_param_copy.bytes_per_frame=display_service->current_buffer->bytes_per_frame;
    display_service->local_param_copy.mode=display_service->current_buffer->mode;
    display_service->local_param_copy.format=display_service->current_buffer->format;
    display_service->local_param_copy.format7_color_mode=display_service->current_buffer->format7_color_mode;
    display_service->local_param_copy.stereo_decoding=display_service->current_buffer->stereo_decoding;
    display_service->local_param_copy.bayer=display_service->current_buffer->bayer;
    display_service->local_param_copy.buffer_image_bytes=display_service->current_buffer->buffer_image_bytes;
    
    // DO SOMETHING
    // if the width is not -1, that is if some image has already reached the thread
    if ((display_service->local_param_copy.width!=-1)&&(size_change!=0)) {
      if (!first_time) {
	//fprintf(stderr,"Not the first time, kill current SDL\n");
	SDLQuit(display_service);
	//usleep(500000);
      }
      //fprintf(stderr,"Init SDL\n");
      SDLInit(display_service);
      //usleep(500000);
    }
  }
}


#endif
