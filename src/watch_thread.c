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

#include "watch_thread.h"

#ifdef HAVE_SDLLIB

extern GtkWidget *main_window;
extern PrefsInfo preferences;
extern camera_t* camera;

int
WatchStartThread(watchthread_info_t* info)
{
  // init threads, mutexes and data
  pthread_mutex_init(&info->mutex_area, NULL);
  pthread_mutex_init(&info->mutex_cancel_watch, NULL);
  pthread_mutex_lock(&info->mutex_area);
  info->draw=0;
  info->mouse_down=0;
  info->crop=0;
  pthread_mutex_unlock(&info->mutex_area);
  pthread_mutex_lock(&info->mutex_cancel_watch);
  info->cancel_watch_req=0;
  pthread_mutex_unlock(&info->mutex_cancel_watch);

  if (pthread_create(&info->thread, NULL, WatchThread, (void*)info))
    return(1);
  else
    return(0);
  
}


void*
WatchThread(void *arg)
{
  int tmp;
  int upper_left[2];
  int lower_right[2];
  int pos[2];
  int size[2];
  watchthread_info_t *info;
 
  info=(watchthread_info_t*)arg;
 
  pthread_mutex_lock(&info->mutex_area);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&info->mutex_area);

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel_watch);
    if (info->cancel_watch_req>0) {
      pthread_mutex_unlock(&info->mutex_cancel_watch);
      return ((void*)1);
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel_watch);
      pthread_mutex_lock(&info->mutex_area);
      
      // do stuff here  -------------------------------------------------------------------------------------
      if (info->crop>0) {
	upper_left[0]=info->upper_left[0];
	upper_left[1]=info->upper_left[1];
	lower_right[0]=info->lower_right[0];
	lower_right[1]=info->lower_right[1];
	
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
	if (info->use_unit_pos>0) {
	  pos[0]=(upper_left[0]/info->f7_step_pos[0])*info->f7_step_pos[0];
	  pos[1]=(upper_left[1]/info->f7_step_pos[1])*info->f7_step_pos[1];
	}
	else {
	  pos[0]=(upper_left[0]/info->f7_step[0])*info->f7_step[0];
	  pos[1]=(upper_left[1]/info->f7_step[1])*info->f7_step[1];
	}
	size[0]=((lower_right[0]-upper_left[0])/info->f7_step[0])*info->f7_step[0];
	size[1]=((lower_right[1]-upper_left[1])/info->f7_step[1])*info->f7_step[1];
	
	//fprintf(stderr,"corners: [%d %d] [%d %d], pos: [%d %d], size[%d %d]\n",
	//      upper_left[0],upper_left[1],lower_right[0],lower_right[1],
	//      pos[0],pos[1],size[0],size[1]);
	SetFormat7Crop(size[0],size[1],pos[0],pos[1]);
	info->crop=0;
      }
      // end stuff -------------------------------------------------------------------------------------
      
      pthread_mutex_unlock(&info->mutex_area);
      usleep(1000);
    }
    
  }
  
}


int
WatchStopThread(watchthread_info_t* info)
{
  // send request for cancellation:
  pthread_mutex_lock(&info->mutex_cancel_watch);
  info->cancel_watch_req=1;
  pthread_mutex_unlock(&info->mutex_cancel_watch);

  // when cancellation occured, join:
  pthread_join(info->thread, NULL);

  return (1);
}


#endif
