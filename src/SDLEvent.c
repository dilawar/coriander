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

#include "SDLEvent.h"

#ifdef HAVE_SDLLIB

extern PrefsInfo preferences;
extern GtkWidget *main_window;
extern watchthread_info_t watchthread_info;
extern camera_t* camera;
extern int WM_cancel_display;
extern cursor_info_t cursor_info;
//extern whitebal_data_t* whitebal_data;

#define YUV2RGB(y, u, v, r, g, b)\
  r = y + ((v*1436) >>10);\
  g = y - ((u*352 + v*731) >> 10);\
  b = y + ((u*1814) >> 10);\
  r = r < 0 ? 0 : r;\
  g = g < 0 ? 0 : g;\
  b = b < 0 ? 0 : b;\
  r = r > 255 ? 255 : r;\
  g = g > 255 ? 255 : g;\
  b = b > 255 ? 255 : b

int
SDLEventStartThread(chain_t *display_service)
{
  displaythread_info_t *info;

  info=(displaythread_info_t*)display_service->data;
  pthread_mutex_init(&info->mutex_cancel_event, NULL);
  pthread_mutex_init(&info->mutex_event, NULL);

  pthread_mutex_lock(&info->mutex_cancel_event);
  info->cancel_event_req=0;
  pthread_mutex_unlock(&info->mutex_cancel_event);
  if (pthread_create(&info->event_thread, NULL, SDLEventThread, (void*)display_service))
    return(1);
  else
    return(0);
}

void*
SDLEventThread(void *arg)
{
  chain_t* display_service;
  displaythread_info_t *info;
 
  display_service=(chain_t*)arg;
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel_event);
    if (info->cancel_event_req>0) {
      pthread_mutex_unlock(&info->mutex_cancel_event);
      return ((void*)1);
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel_event);

      pthread_mutex_lock(&info->mutex_event);
      if (!SDLHandleEvent(display_service)) {
	pthread_mutex_unlock(&info->mutex_event);
	// SDL_QUIT called, close display thread
	DisplayStopThread(camera);
	// the following is only for the display_service button to be updated.
	WM_cancel_display=1;
	break;
      }
      else {
	pthread_mutex_unlock(&info->mutex_event);
	usleep(EVENTS_SLEEP_MS * 1000);
      }
    }
  }
  return ((void*)0);
}

void
SDLEventStopThread(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;
  // send request for cancellation:
  pthread_mutex_lock(&info->mutex_cancel_event);
  info->cancel_event_req=1;
  pthread_mutex_unlock(&info->mutex_cancel_event);
  
  // when cancellation occured, join:
  pthread_join(info->event_thread, NULL);
}


int
SDLHandleEvent(chain_t *display_service)
{
  displaythread_info_t *info;
  SDL_Event event;

  info=(displaythread_info_t*)display_service->data;

  while ( SDL_PollEvent(&event) ) {
    pthread_mutex_lock(&display_service->mutex_data);
    switch(event.type) {
    case SDL_VIDEORESIZE:
      SDLResizeDisplay(display_service, event.resize.w, event.resize.h);
      break;
    case SDL_KEYDOWN:
      OnKeyPressed(display_service,event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_KEYUP:
      OnKeyReleased(display_service,event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_MOUSEBUTTONDOWN:
      OnMouseDown(display_service,event.button.button, event.button.x, event.button.y);
      break;
    case SDL_MOUSEBUTTONUP:
      OnMouseUp(display_service,event.button.button, event.button.x, event.button.y);
      break;
    case SDL_MOUSEMOTION:
      OnMouseMotion(display_service, event.motion.x, event.motion.y);
      break;
    case SDL_QUIT:
      pthread_mutex_unlock(&display_service->mutex_data);
      return(0);
      break;
    }
    pthread_mutex_unlock(&display_service->mutex_data);
  }
  return(1);
}


void
OnKeyPressed(chain_t *display_service, int key, int mod)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  switch (key) {
  case SDLK_n:
    // set display to normal size
    SDLResizeDisplay(display_service, display_service->current_buffer->width, display_service->current_buffer->height);
    break;
  case SDLK_f:
    // toggle fullscreen mode
    SDL_WM_ToggleFullScreen(info->sdlvideo);
    break;
  case SDLK_c:
    // crop image
    SDLCropImage(display_service);
    break;
  case SDLK_m:
    // set F7 image to max size
    SDLSetMaxSize(display_service);
    break;
  case SDLK_GREATER:
    // image size *2
    if (mod&(SDLK_LSHIFT|SDLK_RSHIFT))
      SDLResizeDisplay(display_service, info->sdlvideorect.w/2, info->sdlvideorect.h/2);
    else
      SDLResizeDisplay(display_service, info->sdlvideorect.w*2, info->sdlvideorect.h*2);
    break;
  case SDLK_LESS:
    // image size /2
    if (mod&(SDLK_LSHIFT|SDLK_RSHIFT))
      SDLResizeDisplay(display_service, info->sdlvideorect.w*2, info->sdlvideorect.h*2);
    else
      SDLResizeDisplay(display_service, info->sdlvideorect.w/2, info->sdlvideorect.h/2);
    break;
  }
      
}

void
OnKeyReleased(chain_t *display_service, int key, int mod)
{

}

void
OnMouseDown(chain_t *display_service, int button, int x, int y)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  switch (button) {
  case SDL_BUTTON_LEFT:
    pthread_mutex_lock(&watchthread_info.mutex_area);
    watchthread_info.draw=1;
    watchthread_info.mouse_down=1;
    // there is some adaptation because the display size can be different
    // from the real image size. (i.e. the image can be resized)
    watchthread_info.upper_left[0]= ((x*display_service->current_buffer->width /info->sdlvideorect.w));
    watchthread_info.upper_left[1]= ((y*display_service->current_buffer->height/info->sdlvideorect.h));
    watchthread_info.lower_right[0]=((x*display_service->current_buffer->width /info->sdlvideorect.w));
    watchthread_info.lower_right[1]=((y*display_service->current_buffer->height/info->sdlvideorect.h));
    pthread_mutex_unlock(&watchthread_info.mutex_area);
    break;
  case SDL_BUTTON_MIDDLE:
    x=x*display_service->current_buffer->width/info->sdlvideorect.w; //rescaling
    y=y*display_service->current_buffer->height/info->sdlvideorect.h;
    // THIS IS ONLY VALID FOR YUYV!!
    cursor_info.col_y=info->sdloverlay->pixels[0][(y*display_service->current_buffer->width+x)*2];
    cursor_info.col_u=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->width+x)>>1)<<2)+1]-127;
    cursor_info.col_v=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->width+x)>>1)<<2)+3]-127;
    YUV2RGB(cursor_info.col_y, cursor_info.col_u, cursor_info.col_v,
	    cursor_info.col_r, cursor_info.col_g, cursor_info.col_b);
    cursor_info.x=x;
    cursor_info.y=y;
    cursor_info.update_req=1;
    break;
  case SDL_BUTTON_RIGHT:
    //whitebal_data->x=x*display_service->current_buffer->width/info->sdlvideorect.w; //rescaling
    //whitebal_data->y=y*display_service->current_buffer->height/info->sdlvideorect.h;
    //whitebal_data->service=display_service;
    //pthread_create(&whitebal_data->thread, NULL, AutoWhiteBalance, (void*)&whitebal_data);
    break;
  default:
    fprintf(stderr,"Bad button ID in SDL!\n");
    break;
  }
}

void
OnMouseUp(chain_t *display_service, int button, int x, int y)
{

  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  switch (button) {
  case SDL_BUTTON_LEFT:
    pthread_mutex_lock(&watchthread_info.mutex_area);
    watchthread_info.mouse_down=0;
    // there is some adaptation because the display size can be different
    // from the real image size. (i.e. the image can be resized)
    //info->lower_right[0]=x*display_service->current_buffer->width/info->sdlvideorect.w;
    //info->lower_right[1]=y*display_service->current_buffer->height/info->sdlvideorect.h;
    pthread_mutex_unlock(&watchthread_info.mutex_area);
    break;
  case SDL_BUTTON_MIDDLE:
    break;
  case SDL_BUTTON_RIGHT:
    break;
  default:
    fprintf(stderr,"Bad button ID in SDL!\n");
    break;
  }
}


void
OnMouseMotion(chain_t *display_service, int x, int y)
{
  displaythread_info_t *info;
  //int col_r,col_g,col_b,col_y,col_u,col_v;
  info=(displaythread_info_t*)display_service->data;
  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (watchthread_info.mouse_down==1) {
    // there is some adaptation because the display size can be different
    // from the real image size. (i.e. the image can be resized)
    watchthread_info.lower_right[0]=x*display_service->current_buffer->width/info->sdlvideorect.w;
    watchthread_info.lower_right[1]=y*display_service->current_buffer->height/info->sdlvideorect.h;
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);
  
}

void
SDLResizeDisplay(chain_t *display_service, int width, int height)
{    
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;
  
  if (preferences.display_keep_ratio>0) {
    // keep aspect ratio and resize following which dimension we change
    if (abs(width-info->sdlvideorect.w) >= (abs(height-info->sdlvideorect.h))) {
      // we changed the width, set height accordingly
      info->sdlvideorect.w = width;
      info->sdlvideorect.h = (width * display_service->current_buffer->height) / display_service->current_buffer->width;
    }
    else {
      // we changed the height, set width accordingly
      info->sdlvideorect.w = (height * display_service->current_buffer->width) / display_service->current_buffer->height;
      info->sdlvideorect.h = height;
    }
  }
  else {
    // bypass aspect keep:
    info->sdlvideorect.w = width;
    info->sdlvideorect.h = height;
  }

  //fprintf(stderr,"Changing display size:\n");

  // Free overlay & video surface
  SDL_FreeYUVOverlay(info->sdloverlay);
  
  //fprintf(stderr,"\tFreed YUV overlay\n");
  SDL_FreeSurface(info->sdlvideo);
  //fprintf(stderr,"\tFreed surface\n");

  // Set requested video mode
  //info->sdlbpp   = SDL_VideoModeOK (info->sdlvideorect.w, info->sdlvideorect.h,
  //				    info->sdlbpp, info->sdlflags);
  info->sdlvideo = SDL_SetVideoMode(info->sdlvideorect.w, info->sdlvideorect.h,
				    info->sdlbpp, info->sdlflags);
  
  if (info->sdlvideo == NULL) {
    MainError(SDL_GetError());
    SDL_Quit();
    return;
  }
  /*
  if (SDL_SetColorKey(info->sdlvideo, SDL_SRCCOLORKEY, 0x0) < 0 ) {
    MainError(SDL_GetError());
  }
  */
  //info->sdlvideo->format->BytesPerPixel=2;
  
  // Create YUV Overlay
  
  info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->width,
					  display_service->current_buffer->height,
					  SDL_YUY2_OVERLAY, info->sdlvideo);
  if (info->sdloverlay==NULL) {
    MainError(SDL_GetError());
    SDL_Quit();
    return;
  }
  
  //fprintf(stderr,"\tOverlay created\n");
}


void
SDLCropImage(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);

  watchthread_info.draw=0;
  if (display_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE)
    watchthread_info.crop=1;

  pthread_mutex_unlock(&watchthread_info.mutex_area);
    
}

void
SDLSetMaxSize(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (display_service->current_buffer->format==FORMAT_SCALABLE_IMAGE_SIZE) {
    watchthread_info.crop=1;
    watchthread_info.upper_left[0]=0;
    watchthread_info.upper_left[1]=0;
    watchthread_info.lower_right[0]=camera->format7_info.mode[camera->misc_info.mode-MODE_FORMAT7_MIN].max_size_x;
    watchthread_info.lower_right[1]=camera->format7_info.mode[camera->misc_info.mode-MODE_FORMAT7_MIN].max_size_y;
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);

}

#endif
