/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
 * Iso video receive, video overlay, and catpure provided by 
 * Dan Dennedy <dan@dennedy.org>
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

#include <gnome.h>
#include "callback_proc.h"
#include "support.h"
#include "update_ranges.h"
#include "definitions.h"
#include "conversions.h"
#include "capture.h"
#include "tools.h"
#include <gdk/gdkprivate.h>
#include "raw1394support.h"
#include <libdc1394/dc1394_control.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_FTPLIB
#include <ftplib.h>
#endif

/* globals required by capture */
unsigned char g_rgb_buffer[1600*1200*3];
dc1394_cameracapture g_single_capture;
gchar g_filename[256];
gchar g_ext[256];
guint gCaptureIdleID;

porthole_info pi;
capture_info ci;
isothread_info it;
activity_info activity;

SelfIdPacket_t *selfid;

extern dc1394_cameracapture *capture;
extern dc1394_miscinfo *misc_info;
extern dc1394_camerainfo *camera;
extern Format7Info *format7_info;
extern GtkWidget *porthole_window;

static inline void
//void
convert_to_rgb( dc1394_cameracapture *capture, unsigned char *src, unsigned char *dest, int mode)
{
  switch(mode) {
    case MODE_160x120_YUV444:
      iyu22rgb( src, dest, capture->frame_width*capture->frame_height);
	  break;
    case MODE_320x240_YUV422:
    case MODE_640x480_YUV422:
    case MODE_800x600_YUV422:
    case MODE_1024x768_YUV422:
    case MODE_1280x960_YUV422:
    case MODE_1600x1200_YUV422:
      uyvy2rgb( src, dest, capture->frame_width*capture->frame_height);
      break;
    case MODE_640x480_YUV411:
      iyu12rgb( src, dest, capture->frame_width*capture->frame_height);
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      memcpy( dest, src, capture->quadlets_per_frame*4);
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
      y2rgb( src, dest, capture->frame_width*capture->frame_height);
	  break;
  }
}

static inline void
//void
convert_to_yuv( dc1394_cameracapture *capture, unsigned char *src, unsigned char *dest, int mode)
{
      switch(mode) {
        case MODE_160x120_YUV444:
          iyu22yuy2( src, dest,
            capture->frame_width*capture->frame_height);
          break;
        case MODE_320x240_YUV422:
        case MODE_640x480_YUV422:
        case MODE_800x600_YUV422:
        case MODE_1024x768_YUV422:
        case MODE_1280x960_YUV422:
        case MODE_1600x1200_YUV422:
          if (pi.xv_format == GUID_UYVY_PACKED)
            memcpy( dest, src, 
              capture->quadlets_per_frame*4);
          else
            uyvy2yuy2( src, dest, 
              capture->frame_width*capture->frame_height);
          break;
        case MODE_640x480_YUV411:
          iyu12yuy2( src, dest, 
            capture->frame_width*capture->frame_height);
          break;
        case MODE_640x480_RGB:
        case MODE_800x600_RGB:
        case MODE_1024x768_RGB:
        case MODE_1280x960_RGB:
        case MODE_1600x1200_RGB:
          rgb2yuy2( src, dest, 
            capture->frame_width*capture->frame_height);
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
          y2yuy2( src, dest, 
            capture->frame_width*capture->frame_height);
          break;
      }
}

gint IsoStartThread(void)
{
  int maxspeed;

  if (activity.isothread==0)
    {
      pthread_mutex_init(&(activity.mutex_capture), NULL);
      pthread_mutex_lock(&(activity.mutex_capture));

      /* currently FORMAT_STILL_IMAGE is not supported*/
      if (misc_info->format == FORMAT_STILL_IMAGE)
	{
	  return(-1);
	}
      
      it.handle = NULL;
      
      // the iso receive handler gets its own raw1394 handle to free the controls
      if ( (it.handle = dc1394_create_handle(0)) < 0)
	{
	  return(-1);
	}

      switch (selfid->packetZero.phySpeed)
	{
	case 1: maxspeed=SPEED_200;break;
	case 2: maxspeed=SPEED_400;break;
	default: maxspeed=SPEED_100;break;
	}
      
      if (dc1394_dma_setup_capture( camera->handle, camera->id, misc_info->iso_channel, 
				    misc_info->format, misc_info->mode, maxspeed,
				    misc_info->framerate, DMA_BUFFERS, capture)
	  == DC1394_SUCCESS)
	{
	  it.receive_method=RECEIVE_METHOD_VIDEO1394;
	}
      else 
	{
	  if ((g_single_capture.capture_buffer == NULL) &&
	      (dc1394_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
				    misc_info->format, misc_info->mode, maxspeed,
				    misc_info->framerate, capture)
	       == DC1394_SUCCESS))
	    {
	      it.receive_method=RECEIVE_METHOD_RAW1394;
	    }
	  else
	    {
	      raw1394_destroy_handle(it.handle);
	      pthread_mutex_unlock(&(activity.mutex_capture));
	      return(-1);
	    }
    
	}
      if (dc1394_start_iso_transmission(camera->handle,camera->id)==DC1394_FAILURE)
	{
	  MainError("Could not start ISO transmission");
	  pthread_mutex_unlock(&(activity.mutex_capture));
	  return(-1);
	}
      else
	if (!pthread_create(&it.thread, NULL, IsoReceiveThread, NULL))
	  {
	    activity.isothread=1;
	    misc_info->is_iso_on=DC1394_TRUE;
	    //fprintf(stderr,"ISO Receive Thread created...\n");
	  }
	else
	  {
	    misc_info->is_iso_on=DC1394_FALSE;
	    pthread_mutex_unlock(&(activity.mutex_capture));
	    return(-1);
	  }
      pthread_mutex_unlock(&(activity.mutex_capture));
    }

  return (1);
}

void* IsoReceiveThreadCleanup(void* arg)
{
  if (it.receive_method == RECEIVE_METHOD_RAW1394)
    dc1394_dma_done_with_buffer(capture);
  pthread_mutex_unlock(&(activity.mutex_capture));
}

void* IsoReceiveThread(void* arg)
{
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

  while (1)
    {
      pthread_testcancel();
      pthread_cleanup_push(IsoReceiveThreadCleanup, NULL);
      pthread_mutex_lock(&(activity.mutex_capture));
      if (it.receive_method == RECEIVE_METHOD_RAW1394)
	dc1394_single_capture(it.handle, capture);
      else
	{
	  dc1394_dma_single_capture(capture);
	  dc1394_dma_done_with_buffer(capture);
	}
      it.buffer_updated=1;
      pthread_mutex_unlock(&(activity.mutex_capture));
      pthread_cleanup_pop(0);
    }
}


gint IsoStopThread(void)
{
  if (activity.isothread>0)
    {
      pthread_cancel(it.thread);
      pthread_join(it.thread, NULL);
      activity.isothread=0;
      pthread_mutex_lock(&(activity.mutex_capture));

      if (it.receive_method == RECEIVE_METHOD_VIDEO1394)
	{
	  dc1394_dma_release_camera(it.handle, capture);
	  dc1394_dma_unlisten(camera->handle, capture);
	}
      else 
	dc1394_release_camera(camera->handle, capture);
      
      raw1394_destroy_handle(it.handle);
      it.handle = NULL;
      it.receive_method = RECEIVE_METHOD_RAW1394;
      pthread_mutex_unlock(&(activity.mutex_capture));
    }

  return (1);
}


gint
PortholeStartThread()
{
  if (activity.porthole==0)
    {
      pthread_mutex_init(&(pi.mutex), NULL);
      pthread_mutex_lock(&(pi.mutex));
      pthread_mutex_lock(&(activity.mutex_capture));
      pi.gdk_buffer = NULL;
      pi.drawable = (GtkWidget*)lookup_widget(porthole_window, "camera_scope");

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
      if (xvInit())
	pi.display_method=DISPLAY_METHOD_XV;
      else
#endif
	{ 
	  pi.display_method=DISPLAY_METHOD_GDK;
	  gdk_rgb_init();
	  pi.gdk_buffer=malloc(capture->frame_width * capture->frame_height * 3);
	}
  
      gtk_widget_set_usize(pi.drawable, capture->frame_width, capture->frame_height);

      pthread_mutex_lock(&activity.mutex_cancel_display);
      activity.cancel_display_thread=0;
      pthread_mutex_unlock(&activity.mutex_cancel_display);

      if (!pthread_create(&pi.thread, NULL, PortholeDisplayThread, misc_info->mode))
	{
	  //fprintf(stderr,"  Display Thread created...\n");
	  activity.porthole=1;
	}
      pthread_mutex_unlock(&activity.mutex_capture);
      pthread_mutex_unlock(&pi.mutex);
    }
    
  return (1);
}


void*
DisplayThreadCleanup(void* arg)
{
  //fprintf(stderr,"cleanup display thread\n");
  pthread_mutex_unlock(&(activity.mutex_capture));
  pthread_mutex_unlock(&(pi.mutex));
}

  
void*
PortholeDisplayThread(void* arg)
{
  int mode;

  mode=(int)arg;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

  while (1)
    {
      //pthread_testcancel();
      //pthread_cleanup_push(DisplayThreadCleanup, NULL);
      pthread_mutex_lock(&(pi.mutex));
      pthread_mutex_lock(&(activity.mutex_capture));
      if (it.buffer_updated)
	{
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
	  if (pi.display_method == DISPLAY_METHOD_XV)
	    {
	      convert_to_yuv(capture, (unsigned char *) capture->capture_buffer, pi.xv_image->data, mode);
	      //fprintf(stderr," YO 4\n");
	      //sleep(1);
	      xvPut();
	      //fprintf(stderr," YO 4b\n");
	      //sleep(1);
	    }
	  else
#endif
	    { //DISPLAY_METHOD_GDK
	      convert_to_rgb(capture, (unsigned char *) capture->capture_buffer, pi.gdk_buffer,mode);
	      gdkPut();
	    }
	  it.buffer_updated=0;
	}
      //fprintf(stderr," YO 5\n");
      //sleep(1);
      pthread_mutex_unlock(&activity.mutex_capture);
      pthread_mutex_unlock(&pi.mutex);
      pthread_mutex_lock(&activity.mutex_cancel_display);
      if (activity.cancel_display_thread>0)
	{
	  pthread_mutex_unlock(&activity.mutex_cancel_display);
	  return ((void*)1);
	}
      else
	pthread_mutex_unlock(&activity.mutex_cancel_display);
      //pthread_cleanup_pop(0);
      
    }
}


gint
PortholeStopThread(void)
{

  if (activity.porthole)
    { //fprintf(stderr,"  Display Thread cancelling...\n");
      pthread_mutex_lock(&activity.mutex_cancel_display);
      activity.cancel_display_thread=1;
      pthread_mutex_unlock(&activity.mutex_cancel_display);
      //pthread_cancel(pi.thread);
      pthread_join(pi.thread, NULL);
      pthread_mutex_lock(&pi.mutex);
      pthread_mutex_lock(&activity.mutex_capture);
      activity.porthole=0;

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

      if (pi.xv_image != NULL)
	{ //fprintf(stderr,"  Display Thread cancelling: Begin XV cleanup...\n");
	  XvStopVideo(pi.display, pi.xv_port, pi.window);
	  //fprintf(stderr,"  Display Thread cancelling: Xv stopped...\n");
	  XShmDetach(pi.display, &pi.xv_shm_info);
	  //fprintf(stderr,"  Display Thread cancelling: Xshm detached...\n");
	  shmdt(pi.xv_shm_info.shmaddr);
	  //fprintf(stderr,"  Display Thread cancelling: shmdt...\n");
	  shmctl(pi.xv_shm_info.shmid, IPC_RMID, 0);
	  //fprintf(stderr,"  Display Thread cancelling: shmctl...\n");
	  XFree(pi.xv_image);
	  //fprintf(stderr,"  Display Thread cancelling: XFree...\n");
	}
#endif
      
      if (pi.gdk_buffer != NULL)
	free(pi.gdk_buffer);
      pthread_mutex_unlock(&pi.mutex);
      pthread_mutex_unlock(&activity.mutex_capture);
      //fprintf(stderr,"  Display Thread cancelled\n");
    }
  return (1);
}



      
#ifdef HAVE_X11_EXTENSIONS_XVLIB_H

gint xvInit(void)
{
  int num_adaptors;
  int num_formats;
  XvImageFormatValues *formats;
  int i,j;
  char xv_name[5];
  XvAdaptorInfo	*adaptor_info;
  XGCValues xgcv;

  GdkWindowPrivate *priv = (GdkWindowPrivate*) pi.drawable->window;
  pi.display = priv->xdisplay;
  pi.window = priv->xwindow;

  pi.xv_port = -1;
  XvQueryAdaptors( pi.display, DefaultRootWindow(pi.display), &num_adaptors, &adaptor_info);

  for(i=0;i<num_adaptors && pi.xv_port < 0;i++)
    {
      formats = XvListImageFormats( pi.display, adaptor_info[i].base_id, &num_formats);
      for (j=0; j<num_formats && pi.xv_port < 0; j++)
	{
	  xv_name[4]=0;
	  memcpy( xv_name, &formats[j].id, 4);
	  if ( (misc_info->mode == MODE_640x480_YUV422 || misc_info->mode == MODE_320x240_YUV422) &&
	       formats[j].id ==  GUID_UYVY_PACKED) 
	    {
	      pi.xv_port = adaptor_info[i].base_id;
	      pi.xv_format = GUID_UYVY_PACKED;
	    }
	  else if (formats[j].id == GUID_YUY2_PACKED)
	    {
	      pi.xv_port = adaptor_info[i].base_id;
	      pi.xv_format = GUID_YUY2_PACKED;
	    }
	}
      XFree(formats);
    }
  
    if (pi.xv_port < 0)
    {
      fprintf(stderr,"No suitable Xv adaptor found, resorting to GDK\n");	
      return(0);
    }
    else
    {
      pi.xv_image = (XvImage *) XvShmCreateImage( pi.display, pi.xv_port,
						  pi.xv_format, 0, capture->frame_width, capture->frame_height, &pi.xv_shm_info);
      pi.xv_shm_info.shmid = shmget( IPC_PRIVATE, pi.xv_image->data_size, IPC_CREAT|0777);
      pi.xv_shm_info.shmaddr = shmat( pi.xv_shm_info.shmid, 0, 0);
      pi.xv_image->data = pi.xv_shm_info.shmaddr;
      pi.xv_shm_info.readOnly = 0;
      if (!XShmAttach( pi.display, &pi.xv_shm_info))
	{
	  fprintf(stderr,"Cannot attach shared memory\n");
	  pi.xv_image = NULL;
	  return (0);
	}
      pi.xv_gc = XCreateGC( pi.display, pi.window, 0, &xgcv);
    }

  return 1;
}

void xvPut(void)
{
  XvShmPutImage( pi.display, pi.xv_port, pi.window, pi.xv_gc, pi.xv_image,
		 0,0, capture->frame_width, capture->frame_height,
		 0,0, capture->frame_width, capture->frame_height, False);
  XFlush( pi.display);
}

#endif

void gdkPut(void)
{
  gdk_draw_rgb_image(pi.drawable->window, pi.drawable->style->fg_gc[pi.drawable->state],
		     0, 0, capture->frame_width, capture->frame_height, GDK_RGB_DITHER_MAX, 
		     pi.gdk_buffer, capture->frame_width*3);
}

gboolean capture_single_frame(void)
{
  int maxspeed;

  if (misc_info->format == FORMAT_STILL_IMAGE)
    return FALSE;

  switch (selfid->packetZero.phySpeed)
    {
    case 1: maxspeed=SPEED_200;break;
    case 2: maxspeed=SPEED_400;break;
    default: maxspeed=SPEED_100;break;
    }
  
  if (it.handle != NULL)
    {
      convert_to_rgb( capture, (unsigned char *) capture->capture_buffer, g_rgb_buffer, misc_info->mode);
      return TRUE;
    
    }
  else if (dc1394_setup_capture(camera->handle, camera->id, misc_info->iso_channel, 
             misc_info->format, misc_info->mode, maxspeed,
             misc_info->framerate, &g_single_capture) == DC1394_SUCCESS) 
    {
       dc1394_single_capture( camera->handle, &g_single_capture);
       convert_to_rgb( &g_single_capture, (unsigned char *) g_single_capture.capture_buffer, g_rgb_buffer, misc_info->mode);
       dc1394_release_camera(camera->handle, &g_single_capture);
       return TRUE;
     }
  return FALSE;
}

gboolean capture_multi_start(gchar *filename)
{
  gchar *tmp;
  int    maxspeed;
  
  if (misc_info->format == FORMAT_STILL_IMAGE)
    return FALSE;
  
  strcpy( g_filename, filename);

  /* make sure a file extension is supplied */
  tmp = strrchr( g_filename, '.');
  if (tmp == NULL) return FALSE;

  if (ci.mode == CAPTURE_MODE_SEQUENTIAL)
  {
    tmp[0] = '\0';
    strcpy( g_ext, strrchr( filename, '.'));
  }
  
  ci.counter = 0;

  switch (selfid->packetZero.phySpeed)
    {
    case 1: maxspeed=SPEED_200;break;
    case 2: maxspeed=SPEED_400;break;
    default: maxspeed=SPEED_100;break;
    }

  /* always setup raw-based capture, so it is ready to fall back to */  
  if (dc1394_setup_capture( camera->handle, camera->id, misc_info->iso_channel, 
       misc_info->format, misc_info->mode, maxspeed, misc_info->framerate, 
       &g_single_capture) == DC1394_SUCCESS)
    return TRUE;
  else 
    return FALSE;
}

void capture_multi_stop(void)
{
  dc1394_release_camera(camera->handle, &g_single_capture);
}

gint capture_idler(gpointer p)
{
  static gchar filename_out[256];

  if (ci.mode == CAPTURE_MODE_OVERWRITE || ci.counter < ULONG_MAX) {
    if (it.handle != NULL) { /* get the porthole's buffer */
      convert_to_rgb( capture, (unsigned char *) capture->capture_buffer, g_rgb_buffer, misc_info->mode);
    } else {  /* fall back on the distinct raw capturer */
      dc1394_single_capture( camera->handle, &g_single_capture);
      convert_to_rgb( &g_single_capture, (unsigned char *) g_single_capture.capture_buffer, g_rgb_buffer, misc_info->mode);
    }

    if (ci.mode == CAPTURE_MODE_SEQUENTIAL)     
    {
      sprintf( filename_out, "%s_%10.10li%s", g_filename, ci.counter++, g_ext);
      save_single_frame( filename_out);
      if (ci.ftp_enable)
        ftp_single_frame( filename_out, ci.ftp_address, ci.ftp_path, filename_out, ci.ftp_user, ci.ftp_passwd);
    } 
    else
    {
      save_single_frame( g_filename);
      if (ci.ftp_enable)
        ftp_single_frame( g_filename, ci.ftp_address, ci.ftp_path, g_filename, ci.ftp_user, ci.ftp_passwd);
    }

    return 1;
    
  } else return 0;
}

void save_single_frame(gchar *filename)
{
  GdkImlibImage *im = NULL;

  switch(misc_info->mode) {
    case MODE_160x120_YUV444:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 160, 120);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_640x480_YUV411:
    case MODE_640x480_YUV422:
    case MODE_640x480_RGB:
    case MODE_640x480_MONO:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 640, 480);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_320x240_YUV422:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 320, 240);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_800x600_YUV422:
    case MODE_800x600_RGB:
    case MODE_800x600_MONO:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 800, 600);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_1024x768_YUV422:
    case MODE_1024x768_RGB:
    case MODE_1024x768_MONO:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 1024, 768);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_1280x960_YUV422:
    case MODE_1280x960_RGB:
    case MODE_1280x960_MONO:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 1280, 960);
      gdk_imlib_save_image(im, filename, NULL);
      break;
    case MODE_1600x1200_YUV422:
    case MODE_1600x1200_RGB:
    case MODE_1600x1200_MONO:
      im=gdk_imlib_create_image_from_data
        (g_rgb_buffer, NULL, 1600, 1200);
       gdk_imlib_save_image(im, filename, NULL);
       break;
  }
  if (im != NULL) gdk_imlib_kill_image(im);
}

gboolean ftp_single_frame(gchar *filename, gchar *host, gchar *path, gchar *dest_filename, gchar *user, gchar *passwd) {
#ifdef HAVE_FTPLIB
    netbuf *ftp_handle;
    char  tmp[256];
    
    FtpInit();
    
    MainStatus("ftp: starting...\n");
    if (!FtpConnect(host, &ftp_handle)) {
        MainError("failed to connect to ftp server.");
        return FALSE;
    }
    sprintf(tmp, "ftp: connected to %s", host);
    MainStatus(tmp);
    
    if (FtpLogin(user, passwd, ftp_handle) != 1) {
        MainError("ftp: login failed.");
        return FALSE;
    }
    sprintf(tmp, "ftp: logged in as %s", user);
    MainStatus(tmp);
    
    if (path != NULL && strcmp(path,"")) {
      if (!FtpChdir(path, ftp_handle))
      {
        MainError("ftp chdir failed");
        return FALSE;
      }
      sprintf(tmp, "ftp: chdir %s", path);
      MainStatus(tmp);
    }
    
    strcpy( tmp, strrchr( dest_filename, '/'));

    if (FtpPut(filename, tmp, FTPLIB_IMAGE, ftp_handle) < 0) {
        MainError("ftp failed to put file.");
        return FALSE;
    }
    sprintf(tmp, "ftp: put %s", tmp);
    MainStatus(tmp);
    
    MainStatus("ftp: done.");
    
    FtpQuit(ftp_handle);
    return TRUE;
#else
    return FALSE;
#endif
}
