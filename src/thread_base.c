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

#include "thread_base.h" 

extern camera_t* camera;
extern camera_t* cameras;

chain_t*
GetService(camera_t* cam, service_t service)
{
  chain_t *chain;

  chain=cam->image_pipe;

  if (chain==NULL) {
    return(NULL);
  }
  else {
    while(chain!=NULL) { 
      pthread_mutex_lock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
      if (service==chain->service) {
	pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	return(chain);
      }
      else {
	pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	chain=chain->next_chain;
      }
    }
  }

  return(NULL);
}


int
RollBuffers(chain_t* chain)
{
  buffer_t* tmp_buffer;
  int new_current=0;
  //fprintf(stderr,"rolling buffers...");
  if (chain->service==SERVICE_ISO) {
    // 1 - 2 - 3 -> 1 - 3 - 2 (publish for next service, ISO only)
    tmp_buffer=chain->current_buffer;
    chain->current_buffer=chain->next_buffer;
    chain->next_buffer=tmp_buffer;
    chain->updated=1;
  }
  else {
    // 1 - 3 - 2 -> 3 - 1 - 2 (get from previous service, other services)
    if (chain->prev_chain!=NULL) { // look for previous chain existance
      pthread_mutex_lock(&chain->prev_chain->mutex_data);
      if (chain->prev_chain->updated>0) { // look for updated picture
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
  }

  //fprintf(stderr,"done: %d\n",new_current);
  return(new_current);
  
}


void
CommonChainSetup(camera_t* cam, chain_t* chain, service_t req_service)
{
  chain_t* probe_chain;

  // no thread tries to access this data before it is connected.
  // It is thus safe not to use mutex here (for local data only of course).
  
  chain->service=req_service;
  
  probe_chain=cam->image_pipe; // set the begin point for search
  if (req_service==SERVICE_ISO) {
    chain->next_chain=probe_chain;// the chain is inserted BEFORE probe_chain
    chain->prev_chain=NULL;
  }
  else {
    if (probe_chain!=NULL) {
      pthread_mutex_lock(&probe_chain->mutex_struct);
      while ((probe_chain->next_chain!=NULL)&&(probe_chain->service<req_service)) {
	pthread_mutex_unlock(&probe_chain->mutex_struct);
	probe_chain=probe_chain->next_chain;
	pthread_mutex_lock(&probe_chain->mutex_struct);
      }
      chain->next_chain=probe_chain->next_chain;// the chain is inserted AFTER probe_chain
      chain->prev_chain=probe_chain;
      pthread_mutex_unlock(&probe_chain->mutex_struct);
    }
    else { // chain is the first one
      chain->next_chain=NULL;
      chain->prev_chain=NULL;
    }
  }

  // allocate buffer structures, NOT IMAGE BUFFERS:
  chain->current_buffer=(buffer_t*)malloc(sizeof(buffer_t));
  chain->next_buffer=(buffer_t*)malloc(sizeof(buffer_t));

  chain->camera=cam;

  InitBuffer(chain->current_buffer);
  InitBuffer(chain->next_buffer);
  InitBuffer(&chain->local_param_copy);

  chain->updated=0;
}


void
InsertChain(camera_t* cam, chain_t* chain)
{

  // we should only use mutex_struct in this function

  // we should now effectively make the break in the pipe:
  if (chain->next_chain!=NULL)
    pthread_mutex_lock(&chain->next_chain->mutex_struct);
  if (chain->prev_chain!=NULL)
    pthread_mutex_lock(&chain->prev_chain->mutex_struct);
  
  if (chain->prev_chain!=NULL)
    chain->prev_chain->next_chain=chain;
  else // we have a new first chain
    cam->image_pipe=chain;

  if (chain->next_chain!=NULL)
    chain->next_chain->prev_chain=chain;
  
  if (chain->prev_chain!=NULL)
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);   

}


void
RemoveChain(camera_t* cam, chain_t* chain)
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
  else // it was the first chain
    cam->image_pipe=chain->next_chain;

  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    chain->next_chain->prev_chain=chain->prev_chain;
  
  // UNLOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);

  //fprintf(stderr,"pipe: 0x%x\n",image_pipes[camera]);

}

void
FreeChain(chain_t* chain)
{

  //fprintf(stderr,"FreeChain...\n");
  if (chain!=NULL) {
    if (chain->data!=NULL)
      free(chain->data);
    if (chain->current_buffer!=NULL) {
      if (chain->current_buffer->image!=NULL) {
	free(chain->current_buffer->image);
      }
      free(chain->current_buffer);
    }
    if (chain->next_buffer!=NULL) {
      if (chain->next_buffer->image!=NULL)
	free(chain->next_buffer->image);
      free(chain->next_buffer);
    }
    free(chain);
  }
  //fprintf(stderr,"done\n");
}


void
convert_to_rgb(buffer_t *buffer, unsigned char *dest)
{
  switch(buffer->buffer_color_mode) {
  case COLOR_FORMAT7_MONO8:
    y2rgb(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV411:
    uyyvyy2rgb(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV422:
    uyvy2rgb(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_YUV444:
    uyv2rgb(buffer->image,dest,buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_RGB8:
    memcpy(dest,buffer->image,3*buffer->width*buffer->height);
    break;
  case COLOR_FORMAT7_MONO16:
    y162rgb(buffer->image,dest,buffer->width*buffer->height,buffer->bpp);
    break;
  case COLOR_FORMAT7_RGB16:
    rgb482rgb(buffer->image,dest,buffer->width*buffer->height);
    break;
  }
}


void
InitBuffer(buffer_t *buffer)
{
  buffer->width=-1;
  buffer->height=-1;
  buffer->bytes_per_frame=-1;
  buffer->mode=-1;
  buffer->bayer=-1;
  buffer->bpp=-1;
  buffer->bayer_pattern=-1;
  buffer->stereo_decoding=-1;
  buffer->format=-1;
  buffer->format7_color_mode=-1;
  buffer->buffer_color_mode=-1;
  buffer->image=NULL;
  buffer->buffer_image_bytes=-1;
}
