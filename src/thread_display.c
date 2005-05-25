/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#include "coriander.h"

gint
DisplayStartThread(camera_t* cam)
{
  chain_t *display_service=NULL;
  displaythread_info_t *info=NULL;

  display_service=GetService(camera,SERVICE_DISPLAY);

  if (display_service==NULL) {// if no display service running...
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
    info->period=cam->prefs.display_period;
    CommonChainSetup(cam, display_service,SERVICE_DISPLAY);
    
    pthread_mutex_lock(&display_service->mutex_struct);
    InsertChain(cam, display_service);
    
    if (pthread_create(&display_service->thread, NULL, DisplayThread, (void*)display_service)) {
      RemoveChain(cam, display_service);
      pthread_mutex_unlock(&display_service->mutex_struct);
      pthread_mutex_unlock(&display_service->mutex_data);
      FreeChain(display_service);
      return(-1);
    }

    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    
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


void*
DisplayThread(void* arg)
{
  chain_t* display_service=NULL;
  displaythread_info_t *info=NULL;
  long int skip_counter;
  float tmp;

  // we should only use mutex_data in this function

  display_service=(chain_t*)arg;
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);
  skip_counter=0;

  // time inits:
  display_service->prev_time = times(&display_service->tms_buf);
  display_service->fps_frames=0;
  display_service->processed_frames=0;

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel);
    
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&display_service->mutex_data);
      if(RollBuffers(display_service)) { // have buffers been rolled?
#ifdef HAVE_SDLLIB
	// check params
	DisplayThreadCheckParams(display_service);
#endif
	if (display_service->current_buffer->width!=-1) {
	  if (skip_counter>=(info->period-1)) {
	    skip_counter=0;
#ifdef HAVE_SDLLIB
	    if (info->sdloverlay!=NULL) {
	      if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
		convert_to_yuv_for_SDL(display_service->current_buffer, info->sdloverlay, preferences.overlay_byte_order);
		
		// informative overlays
		SDLDisplayArea(display_service);
		SDLDisplayPattern(display_service);
		
		SDL_UnlockYUVOverlay(info->sdloverlay);
		SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);
		
		info->redraw_prev_time=times(&info->redraw_tms_buf);
	      }
	    }
#endif
	    display_service->fps_frames++;
	    display_service->processed_frames++;
	  }
	  else { //
	    if (display_service->camera->prefs.display_redraw==DISPLAY_REDRAW_ON) {
	      ConditionalTimeoutRedraw(display_service);
	    }
	    skip_counter++;
	  }
	  // FPS display:
	  display_service->current_time=times(&display_service->tms_buf);

	  tmp=(float)(display_service->current_time-display_service->prev_time)/sysconf(_SC_CLK_TCK);
	  if (tmp==0)
	    display_service->fps=fabs(0.0);
	  else
	    display_service->fps=fabs((float)display_service->fps_frames/tmp);
	}
	pthread_mutex_unlock(&display_service->mutex_data);
      }
      else { //
	if (display_service->camera->prefs.display_redraw==DISPLAY_REDRAW_ON) {
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
    if (interval>(1.0/service->camera->prefs.display_redraw_rate)) { // redraw e.g. 4 times per second
#ifdef HAVE_SDLLIB
      if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
	//MainStatus("Conditional display redraw");
	convert_to_yuv_for_SDL(service->current_buffer, info->sdloverlay, preferences.overlay_byte_order);
	SDLDisplayArea(service);
	SDL_UnlockYUVOverlay(info->sdloverlay);
	SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);
      }
#endif
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
    info=(displaythread_info_t*)display_service->data;
    
    // send request for cancellation:
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    // when cancellation occured, join:
    pthread_join(display_service->thread, NULL);
    
    pthread_mutex_lock(&display_service->mutex_data);
    pthread_mutex_lock(&display_service->mutex_struct);
    RemoveChain(cam,display_service);
#ifdef HAVE_SDLLIB
    SDLQuit(display_service);
#endif
    
    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    FreeChain(display_service);
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
  //SDL_Surface *icon_surface;

  info->sdlbpp=16;
  info->sdlflags=SDL_ANYFORMAT | SDL_RESIZABLE;

  // Initialize the SDL library (video subsystem)
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
    ErrorPopup("Couldn't initialize SDL video subsystem");
    return(0);
  }
  
  sdl_videoinfo = SDL_GetVideoInfo();
  
  if ((xvinfo.max_width!=-1)&&(xvinfo.max_height!=-1)) {
    // if the XV area is too small, we use software accelleration
    if ((xvinfo.max_width<display_service->current_buffer->width)||
	(xvinfo.max_height<display_service->current_buffer->height)) {
      //fprintf(stderr,"Using SW surface\n");
      info->sdlflags|= SDL_SWSURFACE;
      info->sdlflags&= ~SDL_HWSURFACE;
      info->sdlflags&= ~SDL_HWACCEL;
    }
    else {
      //fprintf(stderr,"Using HW surface\n");
      info->sdlflags|= SDL_HWSURFACE;
      info->sdlflags|= SDL_HWACCEL;
      info->sdlflags&= ~SDL_SWSURFACE;
    }
  }
  else {
    // try HW accel and pray...
    info->sdlflags|= SDL_SWSURFACE;
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
  }

  modes=SDL_ListModes(NULL,info->sdlflags);
  if (modes!=(SDL_Rect**)-1) {
    // not all resolutions are OK for this video card. For safety we switch to software accel
    MainStatus("No SDL mode available with hardware accel. Trying without HWSURFACE");
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
    modes=SDL_ListModes(NULL,info->sdlflags);
    if (modes!=(SDL_Rect**)-1) {
      ErrorPopup("No video modes available, even without hardware acceleration. Can't start SDL!");
      SDL_Quit();
      return(0);
    }
  }

  /*
  // set coriander icon
  icon_surface=SDL_CreateRGBSurfaceFrom((void*)coriander_logo_xpm)
  SDL_WM_SetIcon(icon_surface,NULL);
  */

  info->sdlvideorect.x=0;
  info->sdlvideorect.y=0;
  info->sdlvideorect.w=display_service->current_buffer->width;
  info->sdlvideorect.h=display_service->current_buffer->height;

  // maximize display size to XV size if necessary
  if ((xvinfo.max_width!=-1)&&(xvinfo.max_height!=-1)) {
    if (info->sdlvideorect.w>xvinfo.max_width) {
      info->sdlvideorect.w=xvinfo.max_width;
    }
    if (info->sdlvideorect.h>xvinfo.max_height) {
      info->sdlvideorect.h=xvinfo.max_height;
    }
  }

  // Set requested video mode
  info->sdlbpp = SDL_VideoModeOK(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags);
  info->sdlvideo = SDL_SetVideoMode(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags);

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
  SDL_WM_SetCaption(camera->prefs.name,camera->prefs.name);

  // this line broke everything for unknown reasons so I just remove it.
  //info->sdlvideo->format->BytesPerPixel=2;

  // Create YUV Overlay
  switch(preferences.overlay_byte_order) {
  case DC1394_BYTE_ORDER_YUYV:
    info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->width,
					    display_service->current_buffer->height,
					    SDL_YUY2_OVERLAY,info->sdlvideo);
    break;
  case DC1394_BYTE_ORDER_UYVY:
    info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->width,
					    display_service->current_buffer->height,
					    SDL_UYVY_OVERLAY,info->sdlvideo);
    break;
  default:
    fprintf(stderr,"Invalid overlay byte order\n");
    break;
  }
  

  if (info->sdloverlay==NULL) {
    MainError(SDL_GetError());
    SDL_Quit();
    return(0);
  }

  SDLEventStartThread(display_service);

  return(1);
}


void
SDLDisplayArea(chain_t *display_service)
{
  displaythread_info_t *info=(displaythread_info_t*)display_service->data;
  unsigned char *pimage;
  int upper_left[2];
  int lower_right[2];
  int width;
  int tmp;
  register int i;
  register int j;

  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (watchthread_info.draw==1) {
    upper_left[0]=watchthread_info.pos[0];
    upper_left[1]=watchthread_info.pos[1];
    lower_right[0]=watchthread_info.pos[0]+watchthread_info.size[0]-1;
    lower_right[1]=watchthread_info.pos[1]+watchthread_info.size[1]-1;
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

    for (i=upper_left[1];i<=lower_right[1];i++) {
      pimage=info->sdloverlay->pixels[0]+i*info->sdloverlay->pitches[0];
      for (j=upper_left[0];j<=lower_right[0];j++) {
	pimage[j*2]=(unsigned char)(255-pimage[j*2]);
      }
    }
  } else {
    pthread_mutex_unlock(&watchthread_info.mutex_area);
  }
}

void
SDLDisplayPattern(chain_t *display_service)
{
  displaythread_info_t *info=(displaythread_info_t*)display_service->data;
  unsigned char *pimage;
  int sx = display_service->current_buffer->width;
  int sy = display_service->current_buffer->height;
  int y,u,v,is,ie,js,je;
  unsigned char block[4];
  register int i;
  register int j;

  is = (7*sx)/16; is = is-is%2;
  ie = (9*sx)/16; ie = ie-ie%2;
  js = (7*sy)/16;
  je = (9*sy)/16;

  RGB2YUV(camera->prefs.overlay_color_r,camera->prefs.overlay_color_g,camera->prefs.overlay_color_b,y,u,v);

  switch(display_service->camera->prefs.overlay_type) {
  case OVERLAY_TYPE_REPLACE:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      block[0]=y;
      block[1]=u;
      block[2]=y;
      block[3]=v;
      break;
    case DC1394_BYTE_ORDER_UYVY:
      block[0]=u;
      block[1]=y;
      block[2]=v;
      block[3]=y;
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order coding: %d\n",preferences.overlay_byte_order);
      break;
    }
    break;
  case OVERLAY_TYPE_RANDOM:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      block[0]=random()/(RAND_MAX/255);
      block[1]=random()/(RAND_MAX/255);
      block[2]=block[0];
      block[3]=random()/(RAND_MAX/255);
      break;
    case DC1394_BYTE_ORDER_UYVY:
      block[0]=random()/(RAND_MAX/255);
      block[1]=random()/(RAND_MAX/255);
      block[2]=random()/(RAND_MAX/255);
      block[3]=block[1];
      break;
    }
    break;
  case OVERLAY_TYPE_INVERT:
    break;
  default:
    fprintf(stderr,"Invalid display overlay type: %d\n",display_service->camera->prefs.overlay_type);
    break;
  }

  pthread_mutex_lock(&watchthread_info.mutex_area);
  pimage=info->sdloverlay->pixels[0];
  switch(display_service->camera->prefs.overlay_pattern) {
  case OVERLAY_PATTERN_OFF:
    break;
  case OVERLAY_PATTERN_RECTANGLE:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=js;
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      j=je;
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage,  ((j*sx+i)<<1));
	INVPIX(pimage, (((j+1)*sx+i)<<1));
      }
      i=is;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
      i=ie;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=js;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      j=je;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j+1)*sx+i)<<1));
      }
      i=is;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
      i=ie;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_SMALL_CROSS:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/2; 
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/2;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_LARGE_CROSS:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/2;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/2;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_GOLDEN_MEAN:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/3;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      j=(2*sy)/3;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
      i=(2*sx)/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/3;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      j=(2*sy)/3;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
      i=(2*sx)/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_IMAGE:
    break;
  case OVERLAY_PATTERN_OVER_UNDEREXPOSED:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[(j*sx+i)<<1]>254)||(pimage[(j*sx+i)<<1]<1)||
		(pimage[(j*sx+i+1)<<1]>254)||(pimage[(j*sx+i+1)<<1]<1)) {
	      INVPIX(pimage, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      else {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[(j*sx+i)<<1]>254)||(pimage[(j*sx+i)<<1]<1)||
		(pimage[(j*sx+i+1)<<1]>254)||(pimage[(j*sx+i+1)<<1]<1)) {
	      REPLPIX(pimage, block, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      break;
    case DC1394_BYTE_ORDER_UYVY:
      if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[((j*sx+i)<<1)+1]>254)||(pimage[((j*sx+i)<<1)+1]<1)||
		(pimage[((j*sx+i+1)<<1)+1]>254)||(pimage[((j*sx+i+1)<<1)+1]<1)) {
	      INVPIX(pimage, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      else {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[((j*sx+i)<<1)+1]>254)||(pimage[((j*sx+i)<<1)+1]<1)||
		(pimage[((j*sx+i+1)<<1)+1]>254)||(pimage[((j*sx+i+1)<<1)+1]<1)) {
	      REPLPIX(pimage, block, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
    }
    break;
  default:
    fprintf(stderr,"Wrong overlay pattern ID\n");
    break;
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);
}

void
SDLQuit(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

#ifdef HAVE_SDLLIB
  // if width==-1, SDL was never initialized so we do nothing
  if (display_service->current_buffer->width!=-1) {
    SDLEventStopThread(display_service);
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
  //int size_change;
  int prev_image_size[2];
  int prev_overlay_size[2];
  info=(displaythread_info_t*)display_service->data;
  
  // copy harmless parameters anyway:
  display_service->local_param_copy.bpp=display_service->current_buffer->bpp;
  display_service->local_param_copy.bayer_pattern=display_service->current_buffer->bayer_pattern;
  if (display_service->current_buffer->width==-1)
    fprintf(stderr,"Error: display size: %dx%d\n",
	    display_service->current_buffer->width,
	    display_service->current_buffer->height);
  /*
  fprintf(stderr,"D:[%d %d] BPF:%lli ColMode:%d\n",
	  display_service->current_buffer->width,
	  display_service->current_buffer->height,
	  display_service->current_buffer->bytes_per_frame,
	  display_service->current_buffer->color_mode);
  */
  // if some parameters changed, we need to restart the display
  if ((display_service->current_buffer->width!=display_service->local_param_copy.width)||
      (display_service->current_buffer->height!=display_service->local_param_copy.height)//||
      //only a change in the image size requires to restart the display
      //(display_service->current_buffer->color_mode!=display_service->local_param_copy.color_mode)||
      //(display_service->current_buffer->bytes_per_frame!=display_service->local_param_copy.bytes_per_frame)||
      // check bayer and stereo decoding
      //(display_service->current_buffer->stereo_decoding!=display_service->local_param_copy.stereo_decoding)||
      //(display_service->current_buffer->bayer!=display_service->local_param_copy.bayer)
      ) {

    fprintf(stderr,"Parameters changed...\n");
    first_time=((display_service->local_param_copy.width==-1)&&(display_service->current_buffer->width!=-1));
    //size_change=((display_service->current_buffer->width!=display_service->local_param_copy.width)||
    //		 (display_service->current_buffer->height!=display_service->local_param_copy.height));
    
    prev_image_size[0]=display_service->local_param_copy.width;
    prev_image_size[1]=display_service->local_param_copy.height;

    // copy all new parameters:
    display_service->local_param_copy.width=display_service->current_buffer->width;
    display_service->local_param_copy.height=display_service->current_buffer->height;
    display_service->local_param_copy.bytes_per_frame=display_service->current_buffer->bytes_per_frame;
    display_service->local_param_copy.stereo_decoding=display_service->current_buffer->stereo_decoding;
    display_service->local_param_copy.bayer=display_service->current_buffer->bayer;
    display_service->local_param_copy.color_mode=display_service->current_buffer->color_mode;
    display_service->local_param_copy.buffer_image_bytes=display_service->current_buffer->buffer_image_bytes;
    
    // DO SOMETHING
    // if the width is not -1, that is if some image has already reached the thread and the size has changed
    if (display_service->local_param_copy.width!=-1) { //&&(size_change!=0)) {
      if (first_time) {
	SDLInit(display_service);
      } else {
	// note: in order to preserve the previous scaling and ratio, the previous parameters are used to
	// determine the new size of the display area
	prev_overlay_size[0]=info->sdlvideorect.w;
	prev_overlay_size[1]=info->sdlvideorect.h;
	watchthread_info.draw=0;
	//SDLEventStopThread(display_service);
	SDLResizeDisplay(display_service,
			 display_service->current_buffer->width*prev_overlay_size[0]/prev_image_size[0],
			 display_service->current_buffer->height*prev_overlay_size[1]/prev_image_size[1]);
	//SDLEventStartThread(display_service);
      }
    }
    fprintf(stderr,"SDL updated.\n");
  }

}

#endif
