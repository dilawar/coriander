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
#include "thread_display.h"
#include "thread_base.h"
#include "math.h"
#include <stdlib.h>

#ifdef HAVE_SDLLIB
#  include "SDL.h"

#include "SDLEvent.h"
#include "support.h"
#include "definitions.h"
#include "preferences.h"
#include "tools.h"
#include "watch_thread.h"

extern PrefsInfo preferences;
extern Format7Info *format7_info;
extern GtkWidget *format7_window;
extern dc1394_camerainfo *camera;
extern dc1394_miscinfo *misc_info;
extern watchthread_info_t watchthread_info;

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

  while (1)
    {
      pthread_mutex_lock(&info->mutex_cancel_event);
      if (info->cancel_event_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_event);
	  return ((void*)1);
	}
      else
	{
	  pthread_mutex_unlock(&info->mutex_cancel_event);
	  pthread_mutex_lock(&info->mutex_event);
	  if (!SDLHandleEvent(display_service)) 
	    {
	      pthread_mutex_unlock(&info->mutex_event);
	      break;
	    }
	  else
	    {
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

  while ( SDL_PollEvent(&event) )
    {
      pthread_mutex_lock(&display_service->mutex_data);
      switch(event.type)
	{
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

  switch (key)
    {
    case SDLK_n:
      // set display to normal size
      SDLResizeDisplay(display_service, display_service->width, display_service->height);
      break;
    case SDLK_f:
      // toggle fullscreen mode
      SDL_WM_ToggleFullScreen(info->SDL_video);
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
	SDLResizeDisplay(display_service, info->SDL_videoRect.w/2, info->SDL_videoRect.h/2);
      else
	SDLResizeDisplay(display_service, info->SDL_videoRect.w*2, info->SDL_videoRect.h*2);
      break;
    case SDLK_LESS:
      // image size /2
      if (mod&(SDLK_LSHIFT|SDLK_RSHIFT))
	SDLResizeDisplay(display_service, info->SDL_videoRect.w*2, info->SDL_videoRect.h*2);
      else
	SDLResizeDisplay(display_service, info->SDL_videoRect.w/2, info->SDL_videoRect.h/2);
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

  switch (button)
    {
    case SDL_BUTTON_LEFT:
      pthread_mutex_lock(&watchthread_info.mutex_area);
      watchthread_info.draw=1;
      watchthread_info.mouse_down=1;
      // there is some adaptation because the display size can be different
      // from the real image size. (i.e. the image can be resized)
      watchthread_info.upper_left[0]= ((x*display_service->width /info->SDL_videoRect.w));
      watchthread_info.upper_left[1]= ((y*display_service->height/info->SDL_videoRect.h));
      watchthread_info.lower_right[0]=((x*display_service->width /info->SDL_videoRect.w));
      watchthread_info.lower_right[1]=((y*display_service->height/info->SDL_videoRect.h));
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
OnMouseUp(chain_t *display_service, int button, int x, int y)
{

  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  switch (button)
    {
    case SDL_BUTTON_LEFT:
      pthread_mutex_lock(&watchthread_info.mutex_area);
      watchthread_info.mouse_down=0;
      // there is some adaptation because the display size can be different
      // from the real image size. (i.e. the image can be resized)
      //info->lower_right[0]=x*display_service->width/info->SDL_videoRect.w;
      //info->lower_right[1]=y*display_service->height/info->SDL_videoRect.h;
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
  info=(displaythread_info_t*)display_service->data;
  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (watchthread_info.mouse_down==1)
    {
      // there is some adaptation because the display size can be different
      // from the real image size. (i.e. the image can be resized)
      watchthread_info.lower_right[0]=x*display_service->width/info->SDL_videoRect.w;
      watchthread_info.lower_right[1]=y*display_service->height/info->SDL_videoRect.h;
    }
  pthread_mutex_unlock(&watchthread_info.mutex_area);
}

void
SDLResizeDisplay(chain_t *display_service, int width, int height)
{    
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;
  
  if (preferences.display_keep_ratio>0)
    {
      // keep aspect ratio and resize following which dimension we change
      if (abs(width-info->SDL_videoRect.w) >= (abs(height-info->SDL_videoRect.h)))
	{
	  // we changed the width, set height accordingly
	  info->SDL_videoRect.w = width;
	  info->SDL_videoRect.h = (width * display_service->height) / display_service->width;
	}
      else
	{
	  // we changed the hieght, set width accordingly
	  info->SDL_videoRect.w = (height * display_service->width) / display_service->height;
	  info->SDL_videoRect.h = height;
	}
    }
  else
    {
      // bypass aspect keep:
      info->SDL_videoRect.w = width;
      info->SDL_videoRect.h = height;
    }
      
  // Free overlay & video surface
  SDL_FreeYUVOverlay(info->SDL_overlay);
  SDL_FreeSurface(info->SDL_video);
  
  // Set requested video mode
  info->SDL_video = SDL_SetVideoMode(info->SDL_videoRect.w,
				     info->SDL_videoRect.h,
				     info->SDL_bpp,
				     info->SDL_flags);
  if (info->SDL_video == NULL)
    {
      SDL_Quit();
      fprintf(stderr,"Error realocating video overlay after resize\n");
    }

  // Create YUV Overlay
  info->SDL_overlay = SDL_CreateYUVOverlay(display_service->width, display_service->height,
					   SDL_UYVY_OVERLAY,info->SDL_video);
  if (info->SDL_overlay == NULL)
    {
      SDL_Quit();
      fprintf(stderr,"Error creating video overlay after resize\n");
    }
}


void
SDLCropImage(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);

  watchthread_info.draw=0;
  if (display_service->format==FORMAT_SCALABLE_IMAGE_SIZE)
    watchthread_info.crop=1;

  pthread_mutex_unlock(&watchthread_info.mutex_area);
    
}

void
SDLSetMaxSize(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (display_service->format==FORMAT_SCALABLE_IMAGE_SIZE)
    {
      watchthread_info.crop=1;
      watchthread_info.upper_left[0]=0;
      watchthread_info.upper_left[1]=0;
      watchthread_info.lower_right[0]=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_x;
      watchthread_info.lower_right[1]=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_y;
    }
  pthread_mutex_unlock(&watchthread_info.mutex_area);

}

#endif
