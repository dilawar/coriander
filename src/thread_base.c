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

#include <gnome.h>
#include <pthread.h>
#include "support.h"
#include "thread_base.h" 
#include "thread_display.h"
#include "thread_iso.h"
#include "thread_ftp.h"
#include "thread_save.h"
#include "conversions.h"

extern chain_t *image_pipe;
extern chain_t **image_pipes;
extern Format7Info *format7_info;
extern dc1394_miscinfo* misc_info;
extern GtkWidget* commander_window;
extern int current_camera;
extern UIInfo *uiinfo;

chain_t*
GetService(service_t service, unsigned int camera)
{
  chain_t  *chain;

  chain=image_pipes[camera];

  if (chain==NULL)
    {
      return(NULL);
    }
  else
    while(chain!=NULL)
      { 
	pthread_mutex_lock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	if (service==chain->service)
	  {
	    pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	    return(chain);
	  }
	else
	  {
	    pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	    chain=chain->next_chain;
	  }
      }
  return(NULL);
}


int
RollBuffers(chain_t* chain)
{
  unsigned char* tmp_buffer;
  int new_current=0;

  if (chain->prev_chain==NULL)
    {
      // 1 - 2 - 3 -> 1 - 3 - 2 (publish for next service)
      //pthread_mutex_lock(&chain->prev_chain->mutex_data);
      tmp_buffer=chain->current_buffer;
      chain->current_buffer=chain->next_buffer;
      chain->next_buffer=tmp_buffer;
      chain->updated=1;
      //pthread_mutex_unlock(&chain->prev_chain->mutex_data);
    }
  else
    {
      // 1 - 3 - 2 -> 3 - 1 - 2 (get from previous service)
      pthread_mutex_lock(&chain->prev_chain->mutex_data);
      if (chain->prev_chain->updated>0)
	{
	  // publish current one to the next chain:
	  tmp_buffer=chain->current_buffer;
	  chain->current_buffer=chain->next_buffer;
	  chain->next_buffer=tmp_buffer;
	  chain->updated=1;

	  // get previous chain image
	  tmp_buffer=chain->current_buffer;
	  chain->current_buffer=chain->prev_chain->next_buffer;
	  chain->prev_chain->next_buffer=tmp_buffer;
	  chain->prev_chain->updated=0;
	  new_current=1;
	}
      else
	new_current=0;

      pthread_mutex_unlock(&chain->prev_chain->mutex_data);
    }

  return(new_current);
  
}


void
CommonChainSetup(chain_t* chain, service_t req_service, unsigned int camera)
{
  chain_t* probe_chain;
  isothread_info_t* info_iso;
  long int buffer_size=0;

  // no thread tries to acess this data before it is connected.
  // It is thus safe not to use mutexes here (for local data only of course).
  
  chain->service=req_service;
  
  probe_chain=image_pipes[camera]; // set the begin point for search
  if (probe_chain!=NULL)
    {
      pthread_mutex_lock(&probe_chain->mutex_struct);
      while ((probe_chain->next_chain!=NULL)&&(probe_chain->service<req_service))
	{
	  pthread_mutex_unlock(&probe_chain->mutex_struct);
	  probe_chain=probe_chain->next_chain;
	  pthread_mutex_lock(&probe_chain->mutex_struct);
	}
      chain->next_chain=probe_chain->next_chain;// the chain is inserted AFTER probe_chain
      chain->prev_chain=probe_chain;
      pthread_mutex_unlock(&probe_chain->mutex_struct);
    }
  else // chain is the first one
    {
      chain->next_chain=NULL;
      chain->prev_chain=NULL;
    }

  if (chain->service==SERVICE_ISO)
    {
      info_iso=(isothread_info_t*)chain->data;
      buffer_size=info_iso->capture.quadlets_per_frame*4;
      chain->width=info_iso->capture.frame_width;
      chain->height=info_iso->capture.frame_height;
      chain->bytes_per_frame=buffer_size;
      chain->mode=misc_info->mode;
      chain->bayer=uiinfo->bayer;
      chain->bayer_pattern=uiinfo->bayer_pattern;
      chain->format=misc_info->format;
      if (misc_info->format==FORMAT_SCALABLE_IMAGE_SIZE)
	chain->format7_color_mode=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].color_coding_id;
      else
	chain->format7_color_mode=-1;
      //fprintf(stderr,"color coding (master): %d\n",format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].color_coding_id);
      //fprintf(stderr,"color coding (copy): %d\n",chain->format7_color_mode);
    }
  else
    { // other type. First check for 'firstness'
      if (probe_chain==NULL)
	{
	  chain->next_chain=NULL;
	  chain->prev_chain=NULL;
	  // we currently make an error here as ISO should be running
	  fprintf(stderr,"ISO thread not running while mounting another thread!\n");
	}
      else
	{ // inherit properties.
	  chain->height=chain->prev_chain->height;
	  chain->width=chain->prev_chain->width;
	  chain->mode=chain->prev_chain->mode;
	  chain->format=chain->prev_chain->format;
	  chain->bytes_per_frame=chain->prev_chain->bytes_per_frame;
	  buffer_size=chain->bytes_per_frame;
	  chain->bayer=chain->prev_chain->bayer;
	  chain->bayer_pattern=chain->prev_chain->bayer_pattern;
	  chain->format7_color_mode=chain->prev_chain->format7_color_mode;
	  //fprintf(stderr,"color coding (slave): %d\n",chain->format7_color_mode);
	}
    }

  if (chain->bayer==NO_BAYER_DECODING)
    {
      chain->current_buffer=(unsigned char*)malloc(buffer_size*sizeof(unsigned char));
      chain->next_buffer=(unsigned char*)malloc(buffer_size*sizeof(unsigned char));
    }
  else // we must allocate a much larger buffer: sx*sy*3 (RGB...)
    {
      chain->current_buffer=(unsigned char*)malloc(chain->width*chain->height*3*sizeof(unsigned char));
      chain->next_buffer=(unsigned char*)malloc(chain->width*chain->height*3*sizeof(unsigned char));
    }
  if ((chain->current_buffer==NULL)||(chain->current_buffer==NULL))
    fprintf(stderr,"Empty buffers allocated!\n");
}


void
InsertChain(chain_t* chain, unsigned int camera)
{

  // we should only use mutex_struct in this function

  if ((chain->next_chain==NULL)&&(chain->prev_chain==NULL))
    {
      // the pipe is empty
      image_pipes[camera]=chain;
    }
  else
    { // we should now effectively make the break in the pipe:
      if (chain->next_chain!=NULL)
	pthread_mutex_lock(&chain->next_chain->mutex_struct);
      if (chain->prev_chain!=NULL)
	pthread_mutex_lock(&chain->prev_chain->mutex_struct);
      
      if (chain->prev_chain!=NULL)
	chain->prev_chain->next_chain=chain;
      if (chain->next_chain!=NULL)
	chain->next_chain->prev_chain=chain;
      
      if (chain->prev_chain!=NULL)
	pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
      if (chain->next_chain!=NULL)
	pthread_mutex_unlock(&chain->next_chain->mutex_struct);
  
    }
}


void
RemoveChain(chain_t* chain, unsigned int camera)
{

  // we should only use mutex_struct in this function

  // LOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_lock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_lock(&chain->next_chain->mutex_struct);

  // note that we want simultaneous lock of the prev AND next chains before we disconnect
  // the current chain.
  
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    chain->prev_chain->next_chain=chain->next_chain;
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    chain->next_chain->prev_chain=chain->prev_chain;
  if ((chain->prev_chain==NULL)&&(chain->next_chain==NULL)) // we are the only element
    {
      image_pipes[camera]=NULL;
    }
  
  // UNLOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);

}

void
FreeChain(chain_t* chain)
{
  if (chain!=NULL)
    {
      if (chain->data!=NULL)
	free(chain->data);
      if (chain->current_buffer!=NULL)
	free(chain->current_buffer);
      if (chain->next_buffer!=NULL)
	free(chain->next_buffer);
      free(chain);
    }
}


void
convert_to_rgb(unsigned char *src, unsigned char *dest, int mode, int width, int height, int f7_colormode, int bayer)
{
  if (bayer==NO_BAYER_DECODING)
    {
      switch(mode)
	{
	case MODE_160x120_YUV444:
	  uyv2rgb(src,dest,width*height);
	  break;
	case MODE_320x240_YUV422:
	case MODE_640x480_YUV422:
	case MODE_800x600_YUV422:
	case MODE_1024x768_YUV422:
	case MODE_1280x960_YUV422:
	case MODE_1600x1200_YUV422:
	  uyvy2rgb(src,dest,width*height);
	  break;
	case MODE_640x480_YUV411:
	  uyyvyy2rgb(src,dest,width*height);
	  break;
	case MODE_640x480_RGB:
	case MODE_800x600_RGB:
	case MODE_1024x768_RGB:
	case MODE_1280x960_RGB:
	case MODE_1600x1200_RGB:
	  memcpy(dest,src,3*width*height);
	  break;
	case MODE_640x480_MONO:
	case MODE_800x600_MONO:
	case MODE_1024x768_MONO:
	case MODE_1280x960_MONO:
	case MODE_1600x1200_MONO:
	  y2rgb(src,dest,width*height);
	  break;
	case MODE_640x480_MONO16:
	case MODE_800x600_MONO16:
	case MODE_1024x768_MONO16:
	case MODE_1280x960_MONO16:
	case MODE_1600x1200_MONO16:
	  y162rgb(src,dest,width*height);
	  break;
	case MODE_FORMAT7_0:
	case MODE_FORMAT7_1:
	case MODE_FORMAT7_2:
	case MODE_FORMAT7_3:
	case MODE_FORMAT7_4:
	case MODE_FORMAT7_5:
	case MODE_FORMAT7_6:
	case MODE_FORMAT7_7:
	  switch (f7_colormode)
	    {
	    case COLOR_FORMAT7_MONO8:
	      y2rgb(src,dest,width*height);
	      break;
	    case COLOR_FORMAT7_YUV411:
	      uyyvyy2rgb(src,dest,width*height);
	      break;
	    case COLOR_FORMAT7_YUV422:
	      uyvy2rgb(src,dest,width*height);
	      break;
	    case COLOR_FORMAT7_YUV444:
	      uyv2rgb(src,dest,width*height);
	      break;
	    case COLOR_FORMAT7_RGB8:
	      memcpy(dest,src,3*width*height);
	      break;
	    case COLOR_FORMAT7_MONO16:
	      y162rgb(src,dest,width*height);
	      break;
	    case COLOR_FORMAT7_RGB16:
	      rgb482rgb(src,dest,width*height);
	      break;
	    }
	  break;
	}
    }
  else // we force RGB mode, thus use memcpy
    memcpy(dest,src,3*width*height);
    
}

void
CleanThreads(clean_mode_t mode)
{
  switch (mode)
    {
    case CLEAN_MODE_UI_UPDATE:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_real")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_ftp")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_save")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_display")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_iso")),FALSE);
      break;
    case CLEAN_MODE_NO_UI_UPDATE:
      RealStopThread();
      FtpStopThread();
      SaveStopThread();
      DisplayStopThread(current_camera);
      IsoStopThread();
      break;
    case CLEAN_MODE_UI_UPDATE_NOT_ISO:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_real")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_ftp")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_save")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_display")),FALSE);
      IsoStopThread();
      break;
      
    }
}
