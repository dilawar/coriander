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

#include <gdk/gdkprivate.h>
#include <libdc1394/dc1394_control.h>
#include <pthread.h>

#ifdef HAVE_SDLLIB
#  include "SDL.h"
#  include "SDLEvent.h"
#endif

#include "support.h"
#include "definitions.h"
#include "tools.h"
#include "preferences.h"
#include "conversions.h"
#include "thread_base.h"
#include "thread_display.h"
#include "thread_iso.h"

extern PrefsInfo preferences;
extern dc1394_miscinfo *misc_info;
extern int current_camera;
extern dc1394_camerainfo *camera;

gint
DisplayStartThread()
{
  chain_t *display_service=NULL;
  displaythread_info_t *info=NULL;

  display_service=GetService(SERVICE_DISPLAY,current_camera);

  if (display_service==NULL)// if no display service running...
    {
      //fprintf(stderr,"No DISPLAY service found, inserting new one\n");
      display_service=(chain_t*)malloc(sizeof(chain_t));
      display_service->data=(void*)malloc(sizeof(displaythread_info_t));
      info=(displaythread_info_t*)display_service->data;
      pthread_mutex_init(&display_service->mutex_struct, NULL);
      pthread_mutex_init(&display_service->mutex_data, NULL);
      pthread_mutex_init(&info->mutex_cancel_display, NULL);

      pthread_mutex_lock(&info->mutex_cancel_display);
      info->cancel_display_req=0;
      pthread_mutex_unlock(&info->mutex_cancel_display);

      pthread_mutex_lock(&display_service->mutex_data);
      info->period=preferences.display_period;
      CommonChainSetup(display_service,SERVICE_DISPLAY,current_camera);

#ifdef HAVE_SDLLIB
      if (!sdlInit(display_service))
#endif
	{
	  MainError("SDL is not available for display. Try Xv or GDK.");
	  
#ifdef HAVE_SDLLIB
	  SDL_FreeYUVOverlay(info->SDL_overlay);
	  SDL_FreeSurface(info->SDL_video);
	  SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
	  pthread_mutex_unlock(&display_service->mutex_data);
	  FreeChain(display_service);
	  return(-1);
	}

      pthread_mutex_unlock(&display_service->mutex_data);

      pthread_mutex_lock(&display_service->mutex_struct);
      InsertChain(display_service,current_camera);
      pthread_mutex_unlock(&display_service->mutex_struct);
      
      pthread_mutex_lock(&display_service->mutex_data);
      pthread_mutex_lock(&display_service->mutex_struct);
      if (pthread_create(&display_service->thread, NULL,
			 DisplayThread, (void*)display_service))
	{
	  RemoveChain(display_service,current_camera);
	  pthread_mutex_unlock(&display_service->mutex_struct);
	  pthread_mutex_unlock(&display_service->mutex_data);
	  FreeChain(display_service);
	  return(-1);
	}
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
  while (1)
    {
      pthread_mutex_lock(&info->mutex_cancel_display);
      if (info->cancel_display_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_display);
	  return ((void*)1);
	}
      else
	{
	  pthread_mutex_unlock(&info->mutex_cancel_display);
	  pthread_mutex_lock(&display_service->mutex_data);
	  if(RollBuffers(display_service)) // have buffers been rolled?
	    {
	      if (skip_counter==(info->period-1))
		{
		  skip_counter=0;
#ifdef HAVE_SDLLIB
		  if (SDL_LockYUVOverlay(info->SDL_overlay) == 0)
		    {
		      convert_to_yuv_for_SDL(display_service->current_buffer,
					     info->SDL_overlay->pixels[0], display_service->mode,
					     display_service->width, display_service->height);
		      SDL_UnlockYUVOverlay(info->SDL_overlay);
		      SDL_DisplayYUVOverlay(info->SDL_overlay, &info->SDL_videoRect);
		    }
#endif
		}
	      else
		skip_counter++;
	      pthread_mutex_unlock(&display_service->mutex_data);
	    }
	  else
	    {
	      pthread_mutex_unlock(&display_service->mutex_data);
	      usleep(THREAD_LOOP_SLEEP_TIME_US);
	    }
	}
    }
}


gint
DisplayStopThread(unsigned int camera)
{
  displaythread_info_t *info;
  chain_t *display_service;
  display_service=GetService(SERVICE_DISPLAY, camera);
  
  if (display_service!=NULL)// if display service running...
    { 
      //fprintf(stderr,"DISPLAY service found, stopping\n");
      info=(displaythread_info_t*)display_service->data;

      // send request for cancellation:
      pthread_mutex_lock(&info->mutex_cancel_display);
      info->cancel_display_req=1;
      pthread_mutex_unlock(&info->mutex_cancel_display);

      // when cancellation occured, join:
      pthread_join(display_service->thread, NULL);

      pthread_mutex_lock(&display_service->mutex_data);
      pthread_mutex_lock(&display_service->mutex_struct);
      RemoveChain(display_service, camera);
#ifdef HAVE_SDLLIB
	  SDLEventStopThread(display_service);
	  SDL_FreeYUVOverlay(info->SDL_overlay);
	  SDL_FreeSurface(info->SDL_video);
	  SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
      pthread_mutex_unlock(&display_service->mutex_struct);
      pthread_mutex_unlock(&display_service->mutex_data);
      FreeChain(display_service);
      //fprintf(stderr," DISPLAY service stopped\n");
    }
  return (1);
}

#ifdef HAVE_SDLLIB

int
sdlInit(chain_t *display_service)
{
  char tmp[STRING_SIZE];
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  info->SDL_bpp=0;
  info->SDL_flags=SDL_ANYFORMAT | SDL_RESIZABLE;

  // Initialize the SDL library (video subsystem)
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
    {
      fprintf(stderr,"Couldn't initialize SDL video subsystem\n");
      return(0);
    }

  // Set requested video mode
  info->SDL_video = SDL_SetVideoMode(display_service->width, display_service->height, 
				     info->SDL_bpp, info->SDL_flags);
  // (0 is bpp)
  if (info->SDL_video == NULL)
    {
      SDL_Quit();
      fprintf(stderr,"Error while getting video surface\n");
      return(0);
    }

  // Hide cursor
  SDL_ShowCursor(1);
  
  // set window title:
  sprintf(tmp,"Node %d: %s %s",camera->id,camera->vendor,camera->model);
  SDL_WM_SetCaption(tmp,"");
  

  // Create YUV Overlay
  info->SDL_overlay = SDL_CreateYUVOverlay(display_service->width, display_service->height, 
					   SDL_UYVY_OVERLAY,info->SDL_video);
  if (info->SDL_overlay == NULL)
    {
      SDL_Quit();
      fprintf(stderr,"Error while creating yuv overlay\n");
      return(0);
    }

  info->SDL_videoRect.x=0;
  info->SDL_videoRect.y=0;
  info->SDL_videoRect.w=display_service->width;
  info->SDL_videoRect.h=display_service->height;

  SDLEventStartThread(display_service);

  return(1);
}

// we should optimize this for RGB too...
void
convert_to_yuv_for_SDL(unsigned char *src, unsigned char *dest, int mode, int width, int height)
{
  switch(mode)
    {
    case MODE_160x120_YUV444:
      uyv2uyvy(src,dest,width*height);
      break;
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      yuyv2uyvy(src,dest,width*height);
      break;
    case MODE_640x480_YUV411:
      uyyvyy2uyvy(src,dest,width*height);
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      rgb2uyvy(src,dest,width*height);
      break;
    case MODE_640x480_MONO16:
    case MODE_800x600_MONO16:
    case MODE_1024x768_MONO16:
    case MODE_1280x960_MONO16:
    case MODE_1600x1200_MONO16:
      y162uyvy(src,dest,width*height);
      break;
    case MODE_640x480_MONO:
    case MODE_800x600_MONO:
    case MODE_1024x768_MONO:
    case MODE_1280x960_MONO:
    case MODE_1600x1200_MONO:
      y2uyvy(src,dest,width*height);
      break;
    case MODE_FORMAT7_0:
    case MODE_FORMAT7_1:
    case MODE_FORMAT7_2:
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      break;
    }
}

#endif
