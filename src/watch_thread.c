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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_SDLLIB

#include "watch_thread.h"

extern PrefsInfo preferences;
extern Format7Info *format7_info;
extern GtkWidget *format7_window;
extern dc1394_camerainfo *camera;
extern dc1394_miscinfo *misc_info;


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
  int state;
  GtkAdjustment* adj;
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
	
	IsoFlowCheck(&state);
	//fprintf(stderr,"ISO check completed\n");
	
	//     WARNING!!
	// the order in which we apply the F7 changes IS IMPORTANT!
	// example: from size=128x128, pos=128x128, we can't go to size=1280x1024, pos=0x0 by first changing the size!!
	// Thus, we need to know what to do first...
	// OR, we can set the position to ZEROxZERO, then change size, then change position!! :-))
	if (dc1394_set_format7_image_position(camera->handle,camera->id, misc_info->mode, 0, 0)!=DC1394_SUCCESS)
	  MainError("Could not set Format7 image position to ZERO!!");
	if ((dc1394_set_format7_image_size(camera->handle,camera->id, misc_info->mode, size[0], size[1])!=DC1394_SUCCESS)||
	    (dc1394_set_format7_image_position(camera->handle,camera->id, misc_info->mode, pos[0], pos[1])!=DC1394_SUCCESS))
	  MainError("Could not set Format7 image size and position");
	//fprintf(stderr,"error setting size/pos.\n");
	else {
	  format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].size_x=size[0];
	  format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].size_y=size[1];
	  format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].pos_x=pos[0];
	  format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].pos_y=pos[1];
	  //fprintf(stderr,"size/pos buffered.\n");
	  
	  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hposition_scale")));
	  adj->upper=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_x-size[0];
	  adj->value=pos[0];
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vposition_scale")));
	  adj->upper=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_y-size[1];
	  adj->value=pos[1];
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hsize_scale")));
	  adj->upper=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_x-pos[0];
	  adj->value=size[0];
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vsize_scale")));
	  adj->upper=format7_info->mode[misc_info->mode-MODE_FORMAT7_MIN].max_size_y-pos[1];
	  adj->value=size[1];
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  //fprintf(stderr,"ranges adjusted\n");
	  
	}
	usleep(100e3);
	IsoFlowResume(&state);
	//fprintf(stderr,"Services restarted\n");
	
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
