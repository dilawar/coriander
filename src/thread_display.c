/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
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
extern GtkWidget *porthole_window;
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
      info->gdk_buffer = NULL;
      info->drawable = (GtkWidget*)lookup_widget(porthole_window, "camera_scope");
      info->display_method=preferences.display_method;
      info->period=preferences.display_period;
      CommonChainSetup(display_service,SERVICE_DISPLAY,current_camera);

      //fprintf(stderr,"display method: %d\n",preferences.display_method);
      switch(preferences.display_method)
	{
	case DISPLAY_METHOD_XV:
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
	  //fprintf(stderr,"setting up XV\n");
	  if (!xvInit(display_service))
#endif
	    {
	      MainError("Xv is not available for display. Try GDK or SDL.");
	      pthread_mutex_unlock(&display_service->mutex_data);
	      FreeChain(display_service);
	      return(-1);
	    }
	  break;
	case DISPLAY_METHOD_GDK:
	  //fprintf(stderr,"setting up GDK\n");
	  info->display_method=DISPLAY_METHOD_GDK;
	  gdk_rgb_init();
	  info->gdk_buffer=malloc(display_service->width*display_service->height *3);
	  break;
	case DISPLAY_METHOD_SDL:
#ifdef HAVE_SDLLIB
	  //fprintf(stderr,"setting up SDL\n");
	  if (!sdlInit(display_service))
	    {
#endif
	      //fprintf(stderr,"SDL setup failed\n");
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
	  break;
	  /*
	case DISPLAY_METHOD_AUTO:
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
	  if (xvInit(display_service))
	    info->display_method=DISPLAY_METHOD_XV;
	  else
#endif
	    { 
	      info->display_method=DISPLAY_METHOD_GDK;
	      gdk_rgb_init();
	      info->gdk_buffer=malloc(display_service->width*display_service->height *3);
	    }
	  */
	}      
      gtk_widget_set_usize(info->drawable,display_service->width,display_service->height);
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
  unsigned long int lumBufferSize, chromBufferSize;
  lumBufferSize = display_service->width*display_service->height; 
  chromBufferSize = lumBufferSize/4;

  // we should only use mutex_data in this function

  display_service=(chain_t*)arg;
  //fprintf(stderr,"Outside loop, locking mutex\n");
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);
  //fprintf(stderr,"Outside loop, mutex unlocked\n");
  skip_counter=0;

  while (1)
    {
      //fprintf(stderr," Locking cancel mutex\n");
      pthread_mutex_lock(&info->mutex_cancel_display);
      if (info->cancel_display_req>0)
	{
	  pthread_mutex_unlock(&info->mutex_cancel_display);
	  //fprintf(stderr," cancel mutex unlocked\n");
	  return ((void*)1);
	}
      else
	{
	  //fprintf(stderr," cancel mutex unlocked\n");
	  pthread_mutex_unlock(&info->mutex_cancel_display);
	  //fprintf(stderr," Locking data mutex\n");
	  pthread_mutex_lock(&display_service->mutex_data);
	  if(RollBuffers(display_service)) // have buffers been rolled?
	    {
	      if (skip_counter==(info->period-1))
		{
		  skip_counter=0;
		  //fprintf(stderr,"ready to display: mode %d\n",info->display_method);
		  switch (info->display_method)
		    {
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
		    case DISPLAY_METHOD_XV:
		      //fprintf(stderr,"Auugh!\n");
		      convert_to_yuv_for_xv(display_service->current_buffer,
					    info->xv_image->data, display_service->mode,
					    display_service->width, display_service->height,
					    info->xv_format);
		      xvPut(display_service);
		      break;
#endif
		    case DISPLAY_METHOD_GDK:
		      //fprintf(stderr,"Auugh!\n");
		      convert_to_rgb(display_service->current_buffer,
				     info->gdk_buffer, display_service->mode,
				     display_service->width, display_service->height,
				     display_service->bytes_per_frame);
		      gdkPut(display_service);
		      break;
		    case DISPLAY_METHOD_SDL:
#ifdef HAVE_SDLLIB
		      //fprintf(stderr,"locking\n");
		      if (SDL_LockYUVOverlay(info->SDL_overlay) == 0)
			{
			  //fprintf(stderr,"converting\n");
			  convert_to_yuv_for_SDL(display_service->current_buffer,
						 info->SDL_overlay->pixels[0], display_service->mode,
						 display_service->width, display_service->height);
			  
			  //fprintf(stderr,"unlocking\n");
			  SDL_UnlockYUVOverlay(info->SDL_overlay);
			  //fprintf(stderr,"displaying\n");
			  SDL_DisplayYUVOverlay(info->SDL_overlay, &info->SDL_videoRect);
			  //fprintf(stderr,"   Done!\n");
			}
#endif
		      break;
		    }
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
      switch (info->display_method)
	{
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
	case DISPLAY_METHOD_XV:
	  if (info->xv_image != NULL)
	    { 
	      XvStopVideo(info->display, info->xv_port, info->window);
	      XShmDetach(info->display, &info->xv_shm_info);
	      shmdt(info->xv_shm_info.shmaddr);
	      shmctl(info->xv_shm_info.shmid, IPC_RMID, 0);
	      XFree(info->xv_image);
	    }
	  break;
#endif
	case DISPLAY_METHOD_GDK:
	  if (info->gdk_buffer != NULL)
	    free(info->gdk_buffer);
	  break;
	case DISPLAY_METHOD_SDL:
#ifdef HAVE_SDLLIB
	  SDL_FreeYUVOverlay(info->SDL_overlay);
	  SDL_FreeSurface(info->SDL_video);
	  SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
	  break;
	}

      pthread_mutex_unlock(&display_service->mutex_struct);
      pthread_mutex_unlock(&display_service->mutex_data);
      FreeChain(display_service);
      //fprintf(stderr," DISPLAY service stopped\n");
    }
  return (1);
}


#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

gint xvInit(chain_t *display_service)
{
  int num_adaptors;
  int num_formats;
  XvImageFormatValues *formats;
  int i,j;
  char xv_name[5];
  XvAdaptorInfo	*adaptor_info;
  XGCValues xgcv;

  displaythread_info_t *info;
  GdkWindowPrivate *priv;

  info=(displaythread_info_t*)display_service->data;

  priv=(GdkWindowPrivate*)info->drawable->window;

  info->display = priv->xdisplay;
  info->window = priv->xwindow;

  info->xv_port = -1;
  XvQueryAdaptors(info->display, DefaultRootWindow(info->display), &num_adaptors, &adaptor_info);

  for(i=0;i<num_adaptors && info->xv_port < 0;i++)
    {
      formats = XvListImageFormats( info->display, adaptor_info[i].base_id, &num_formats);
      for (j=0; j<num_formats && info->xv_port < 0; j++)
	{
	  xv_name[4]=0;
	  memcpy( xv_name, &formats[j].id, 4);
	  if (((misc_info->mode == MODE_640x480_YUV422) ||
	       (misc_info->mode == MODE_320x240_YUV422)) &&
	      (formats[j].id ==  GUID_UYVY_PACKED))
	    {
	      info->xv_port = adaptor_info[i].base_id;
	      info->xv_format = GUID_UYVY_PACKED;
	    }
	  else if (formats[j].id == GUID_YUY2_PACKED)
	    {
	      info->xv_port = adaptor_info[i].base_id;
	      info->xv_format = GUID_YUY2_PACKED;
	    }
	}
      XFree(formats);
    }
  
  if (info->xv_port < 0)
    {
      fprintf(stderr,"No suitable Xv adaptor found, resorting to GDK\n");	
      return(0);
    }
  else
    {
      info->xv_image = (XvImage *) XvShmCreateImage(info->display, info->xv_port,info->xv_format, 0,
						    display_service->width, display_service->height,
						    &info->xv_shm_info);
      info->xv_shm_info.shmid = shmget( IPC_PRIVATE, info->xv_image->data_size, IPC_CREAT|0777);
      info->xv_shm_info.shmaddr = shmat( info->xv_shm_info.shmid, 0, 0);
      info->xv_image->data = info->xv_shm_info.shmaddr;
      info->xv_shm_info.readOnly = 0;
      if (!XShmAttach( info->display, &info->xv_shm_info))
	{
	  fprintf(stderr,"Cannot attach shared memory\n");
	  info->xv_image = NULL;
	  return (0);
	}
      info->xv_gc = XCreateGC( info->display, info->window, 0, &xgcv);
    }
  
  return 1;
}

void xvPut(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  XvShmPutImage(info->display, info->xv_port, info->window, info->xv_gc, info->xv_image,
		0,0, display_service->width, display_service->height,
		0,0, display_service->width, display_service->height, False);
  XFlush(info->display);
}

void
convert_to_yuv_for_xv(unsigned char *src, unsigned char *dest, int mode, int width, int height, int xv_format)
{
  switch(mode)
    {
    case MODE_160x120_YUV444:
      iyu22yuy2(src,dest,width*height);
      break;
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      if (xv_format == GUID_UYVY_PACKED)
	memcpy(dest,src,width*height*2);
      else
	uyvy2yuy2(src,dest,width*height);
      break;
    case MODE_640x480_YUV411:
      iyu12yuy2(src,dest,width*height);
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      rgb2yuy2(src,dest,width*height);
      break;
    case MODE_640x480_MONO:
    case MODE_800x600_MONO:
    case MODE_1024x768_MONO:
    case MODE_1280x960_MONO:
    case MODE_1600x1200_MONO:
    case MODE_FORMAT7_0:
    case MODE_FORMAT7_1:
    case MODE_FORMAT7_2:
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      y2yuy2(src,dest,width*height);
      break;
    }
}


#endif

void gdkPut(chain_t *display_service)
{
  //fprintf(stderr,"putting GDK frame...");
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  gdk_draw_rgb_image(info->drawable->window, info->drawable->style->fg_gc[info->drawable->state],
		     0, 0, display_service->width, display_service->height, GDK_RGB_DITHER_MAX, 
		     info->gdk_buffer, display_service->width*3);
  //fprintf(stderr," done.\n");
}

#ifdef HAVE_SDLLIB

int
sdlInit(chain_t *display_service)
{
  char tmp[256];
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  // Initialize the SDL library (video subsystem)
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
    {
      fprintf(stderr,"Couldn't initialize SDL video subsystem\n");
      return(0);
    }

  // Set requested video mode
  info->SDL_video = SDL_SetVideoMode(display_service->width, display_service->height, 0, info->SDL_flags);
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
					   GetSDLVideoMode(display_service->mode),
					   info->SDL_video);
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

  return(1);
}

int
GetSDLVideoMode(int mode)
{
  switch(mode)
    {
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      return(SDL_UYVY_OVERLAY);
      break;
    case MODE_160x120_YUV444:
    case MODE_640x480_YUV411:
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
    case MODE_640x480_MONO:
    case MODE_800x600_MONO:
    case MODE_1024x768_MONO:
    case MODE_1280x960_MONO:
    case MODE_1600x1200_MONO:
    case MODE_FORMAT7_0:
    case MODE_FORMAT7_1:
    case MODE_FORMAT7_2:
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      return(SDL_YUY2_OVERLAY);
      break;
    }

  return(-1);
}

// we should optimize this for RGB too...
void
convert_to_yuv_for_SDL(unsigned char *src, unsigned char *dest, int mode, int width, int height)
{
  switch(mode)
    {
    case MODE_160x120_YUV444:
      iyu22yuy2(src,dest,width*height);
      break;
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      uyvy2yuy2(src,dest,width*height);
      //memcpy(dest,src,width*height*2);
      break;
    case MODE_640x480_YUV411:
      iyu12yuy2(src,dest,width*height);
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      rgb2yuy2(src,dest,width*height);
      break;
    case MODE_640x480_MONO:
    case MODE_800x600_MONO:
    case MODE_1024x768_MONO:
    case MODE_1280x960_MONO:
    case MODE_1600x1200_MONO:
    case MODE_FORMAT7_0:
    case MODE_FORMAT7_1:
    case MODE_FORMAT7_2:
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      y2yuy2(src,dest,width*height);
      break;
    }
}

#endif
