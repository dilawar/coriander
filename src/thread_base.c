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
extern dc1394_miscinfo* misc_info;
extern GtkWidget* commander_window;

chain_t*
GetService(service_t service)
{
  chain_t  *chain;

  chain=image_pipe;

  if (chain==NULL)
    {
      //fprintf(stderr,"No service: empty chain\n");
      return(NULL);
    }
  else
    while(chain!=NULL)
      { 
	pthread_mutex_lock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	//fprintf(stderr,"service probed: %d\n",chain->service);
	if (service==chain->service)
	  {
	    //fprintf(stderr,"service found\n");
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

  // 1 - 2 - 3 -> 1 - 3 - 2 (publish for next service)
  // no locking necessary, only dealing with own data
  tmp_buffer=chain->current_buffer;
  chain->current_buffer=chain->next_buffer;
  chain->next_buffer=tmp_buffer;
  chain->updated=1;
  // 1 - 3 - 2 -> 3 - 1 - 2 (get from previous service)
  // locking necessary: we mess with prev. chain data
  if (chain->prev_chain!=NULL)
    {
      pthread_mutex_lock(&chain->prev_chain->mutex_data);
      if (chain->prev_chain->updated>0)
	{
	  tmp_buffer=chain->current_buffer;
	  chain->current_buffer=chain->prev_chain->next_buffer;
	  chain->prev_chain->next_buffer=tmp_buffer;
	  chain->prev_chain->updated=0;
	  new_current=1;
	}
      else
	{
	  new_current=0;
	  //fprintf(stderr,"client too fast!\n");
	}
      pthread_mutex_unlock(&chain->prev_chain->mutex_data);
    }
  return(new_current);
  
}


void
CommonChainSetup(chain_t* chain, service_t req_service)
{
  chain_t* probe_chain;
  isothread_info_t* info_iso;
  long int buffer_size=0;

  // no thread tries to acess this data before it is connected.
  // It is thus safe not to use mutexes here (for local data only of course).
  
  chain->service=req_service;
  
  probe_chain=image_pipe; // set the begin point for search
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
      //fprintf(stderr,"No break point: empty pipe\n");
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
	  chain->bytes_per_frame=chain->prev_chain->bytes_per_frame;
	  buffer_size=chain->bytes_per_frame;
	}
    }

  chain->current_buffer=(unsigned char*)malloc(buffer_size*sizeof(unsigned char));
  chain->next_buffer=(unsigned char*)malloc(buffer_size*sizeof(unsigned char));

  if ((chain->current_buffer==NULL)||(chain->current_buffer==NULL))
    fprintf(stderr,"Empty buffers allocated!\n");
}


void
InsertChain(chain_t* chain)
{

  // we should only use mutex_struct in this function

  //fprintf(stderr,"Inserting chain\n");
  if ((chain->next_chain==NULL)&&(chain->prev_chain==NULL))
    {
      //fprintf(stderr," Insert chain: empty pipe\n");
      // the pipe is empty
      image_pipe=chain;
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
  
      //fprintf(stderr," Chain inserted\n");
    
    }

}


void
RemoveChain(chain_t* chain)
{

  // we should only use mutex_struct in this function

  //fprintf(stderr,"Removing...\n");
  // LOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_lock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_lock(&chain->next_chain->mutex_struct);

  //fprintf(stderr," Removing: mutex locked\n");
  // note that we want simultaneous lock of the prev AND next chains before we disconnect
  // the current chain.
  
  //fprintf(stderr," Removing: cutting\n");
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    chain->prev_chain->next_chain=chain->next_chain;
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    chain->next_chain->prev_chain=chain->prev_chain;
  if ((chain->prev_chain==NULL)&&(chain->next_chain==NULL)) // we are the only element
    {
      //fprintf(stderr,"Only element removed, image_pipe=NULL\n");
      image_pipe=NULL;
    }
  
  // UNLOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);
  //fprintf(stderr," Removing: mutex unlocked\n");

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
convert_to_rgb(unsigned char *src, unsigned char *dest, int mode, int width, int height, long int bytes_per_frame)
{
  switch(mode)
    {
    case MODE_160x120_YUV444:
      iyu22rgb(src,dest,width*height);
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
      iyu12rgb(src,dest,width*height);
      break;
    case MODE_640x480_RGB:
    case MODE_800x600_RGB:
    case MODE_1024x768_RGB:
    case MODE_1280x960_RGB:
    case MODE_1600x1200_RGB:
      memcpy(dest,src,bytes_per_frame);
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
      y2rgb(src,dest,width*height);
      break;
    }
}

void
CleanThreads(clean_mode_t mode)
{
  switch (mode)
    {
    case CLEAN_MODE_UI_UPDATE:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_ftp")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_save")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_display")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_iso")),FALSE);
      //fprintf(stderr,"CLEAN_MODE_UI_UPDATE\n");
      break;
    case CLEAN_MODE_NO_UI_UPDATE:
      FtpStopThread();
      SaveStopThread();
      DisplayStopThread();
      IsoStopThread();
      //fprintf(stderr,"CLEAN_MODE_NO_UI_UPDATE\n");
      break;
    case CLEAN_MODE_UI_UPDATE_NOT_ISO:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_ftp")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_save")),FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,
								   "service_display")),FALSE);
      IsoStopThread();
      //fprintf(stderr,"CLEAN_MODE_UI_UPDATE_NOT_ISO\n");
      break;
      
    }
}
