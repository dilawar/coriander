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

#include "camera.h"

extern camera_t* camera;
extern camera_t* cameras;

camera_t*
NewCamera(void) {

  return((camera_t*)malloc(sizeof(camera_t)));

}

void
GetCameraData(raw1394handle_t handle, nodeid_t node, camera_t* camera) {

  if (dc1394_get_camera_info(handle, node, &camera->camera_info)!=DC1394_SUCCESS)
    MainError("Could not get camera basic information!");
  if (dc1394_get_camera_misc_info(handle, camera->camera_info.id, &camera->misc_info)!=DC1394_SUCCESS)
    MainError("Could not get camera misc information!");
  if (dc1394_get_camera_feature_set(handle, camera->camera_info.id, &camera->feature_set)!=DC1394_SUCCESS)
    MainError("Could not get camera feature information!");
  GetFormat7Capabilities(handle, camera->camera_info.id, &camera->format7_info);
  camera->image_pipe=NULL;
  pthread_mutex_lock(&camera->uimutex);
  camera->want_to_display=0;
  camera->bayer=NO_BAYER_DECODING;
  camera->stereo=NO_STEREO_DECODING;
  camera->bpp=8;
  pthread_mutex_unlock(&camera->uimutex);

}

void
AppendCamera(camera_t* camera) {

  camera_t* ptr;

  // if first camera:
  if (cameras==NULL) {
    cameras=camera;
    camera->prev=NULL;
    camera->next=NULL;
  }
  else {
    ptr=cameras; 
    while(ptr->next!=NULL) {
      ptr=ptr->next;
    }
    ptr->next=camera;
    camera->prev=ptr;
    camera->next=NULL;
  }
}

void
SetCurrentCamera(u_int64_t guid) {

  camera_t* ptr;
  ptr=cameras;

  while (ptr->camera_info.euid_64!=guid) {
    ptr=ptr->next;
  }
  camera=ptr;
}

void
RemoveCamera(u_int64_t guid) {

  camera_t* ptr;
  ptr=cameras;

  while(ptr->camera_info.euid_64!=guid) {
    ptr=ptr->next;
  }
  
  ptr->prev->next=ptr->next;
  ptr->next->prev=ptr->prev;

  FreeCamera(ptr);
}

void
FreeCamera(camera_t* camera) {

  free(camera);
  camera=NULL;

}
