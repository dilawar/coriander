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

void
GetCameraNodes(BusInfo_t* bi) {

  raw1394handle_t tmp_handle;
  int port;

  tmp_handle=dc1394_create_handle(0); // start with port 0
  bi->card_found=0;
  bi->camera_num=0;

  if (tmp_handle!=NULL) {
    bi->port_num=raw1394_get_port_info(tmp_handle, NULL, 0);
    dc1394_destroy_handle(tmp_handle);
    bi->camera_nodes=(nodeid_t**)malloc(bi->port_num*sizeof(nodeid_t*));
    bi->port_camera_num=(int*)malloc(bi->port_num*sizeof(int));
    bi->handles=(raw1394handle_t *)malloc(bi->port_num*sizeof(raw1394handle_t));
    
    for (port=0;port<bi->port_num;port++) {
      // get a handle to the current interface card
      bi->handles[port]=dc1394_create_handle(port);
      if (bi->handles[port]!=0) { // if the card is present
	bi->card_found=1;
	// probe the IEEE1394 bus for DC camera:
	bi->camera_nodes[port]=dc1394_get_camera_nodes(bi->handles[port], &(bi->port_camera_num[port]), 0); // 0 not to show the cams.
	bi->camera_num+=bi->port_camera_num[port];
      }
    }
  }
}

void
GetCamerasInfo(BusInfo_t* bi) {

  int i;
  int port;
  camera_t* camera_ptr;
  for (port=0;port<bi->port_num;port++) {
    if (bi->handles[port]!=0) {
      for (i=0;i<bi->port_camera_num[port];i++) {
	camera_ptr=NewCamera();
	GetCameraData(bi->handles[port], bi->camera_nodes[port][i], camera_ptr);
	AppendCamera(camera_ptr);
	//fprintf(stderr,"Got camera info\n");
      }
    }
  }
}

camera_t*
NewCamera(void) {
  camera_t* cam;

  cam=(camera_t*)malloc(sizeof(camera_t));
  pthread_mutex_init(&cam->uimutex, NULL);
  return cam;

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
  //fprintf(stderr,"Got camera data\n");
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
  //fprintf(stderr,"removing camera # 0x%llx\n",guid);
  ptr=cameras;
  while (ptr->camera_info.euid_64!=guid) {
    ptr=ptr->next;
  }
  //fprintf(stderr,"\tcamera ptr: 0x%x\n",ptr);

  V4lStopThread(ptr);
  FtpStopThread(ptr);
  SaveStopThread(ptr);
  DisplayStopThread(ptr);
  IsoStopThread(ptr);
  //fprintf(stderr,"\tthreads cleared\n");

  if (ptr->prev!=NULL) // not first camera?
    ptr->prev->next=ptr->next;
  else {
    cameras=ptr->next;
    if (ptr->next!=NULL)
      ptr->next->prev=NULL;
  }

  if (ptr->next!=NULL) // not last camera?
    ptr->next->prev=ptr->prev;
  else {
    if (ptr->prev!=NULL)
      ptr->prev->next=NULL;
  }

  //fprintf(stderr,"\tstruct bypassed\n");

  FreeCamera(ptr);
  //fprintf(stderr,"\tcamera freed\n");

}

void
FreeCamera(camera_t* camera) {

  free(camera);
  camera=NULL;

}
