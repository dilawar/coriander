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

void
GetCameraNodes(BusInfo_t* bi) {

  raw1394handle_t tmp_handle;
  int port;

  tmp_handle=raw1394_new_handle();
  bi->card_found=0;
  bi->camera_num=0;

  if (tmp_handle!=NULL) {
    bi->port_num=raw1394_get_port_info(tmp_handle, NULL, 0);
    //fprintf(stderr,"%d port(s) found\n",bi->port_num);
    raw1394_destroy_handle(tmp_handle);

    if ((bi->camera_nodes!=NULL)&&(bi->port_camera_num!=NULL)&&(bi->handles!=NULL)) {
      for (port=0;port<bi->port_num;port++)
	free(bi->camera_nodes[port]);
      free(bi->camera_nodes);
      free(bi->port_camera_num);
      free(bi->handles);
      bi->camera_nodes=NULL;
      bi->port_camera_num=NULL;
      bi->handles=NULL;
    }

    if ((bi->camera_nodes==NULL)&&(bi->port_camera_num==NULL)&&(bi->handles==NULL)) {
      bi->camera_nodes=(nodeid_t**)malloc(bi->port_num*sizeof(nodeid_t*));
      bi->port_camera_num=(int*)malloc(bi->port_num*sizeof(int));
      bi->handles=(raw1394handle_t *)malloc(bi->port_num*sizeof(raw1394handle_t));
    }
    else {
      fprintf(stderr,"ERROR allocating bus info structures!\n");
    }
    
    for (port=0;port<bi->port_num;port++) {
      // get a handle to the current interface card
      bi->handles[port]=dc1394_create_handle(port);
      if (bi->handles[port]!=0) { // if the card is present
	bi->card_found=1;
	// set bus reset handler
	raw1394_set_bus_reset_handler(bi->handles[port], bus_reset_handler);
	// probe the IEEE1394 bus for DC camera:
	bi->camera_nodes[port]=dc1394_get_camera_nodes(bi->handles[port], &(bi->port_camera_num[port]), 0); // 0 not to show the cams.
	//fprintf(stderr,"There is %d cameras on port %d\n",bi->port_camera_num[port],port);
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
  camera_t* tmp;
  for (port=0;port<bi->port_num;port++) {
    if (bi->handles[port]!=0) {
      for (i=0;i<bi->port_camera_num[port];i++) {
	camera_ptr=NewCamera();
	GetCameraData(port, bi->camera_nodes[port][i], camera_ptr);

	// check that the camera is not yet found through another interface card (for strange bus topologies):
	tmp=cameras;
	while (tmp!=NULL) {
	  if (tmp->camera_info.euid_64==camera_ptr->camera_info.euid_64) {
	    // the camera is already there. don't append.
	    FreeCamera(camera_ptr);
	    break;
	  }
	  else {
	    tmp=tmp->next;
	  }
	}
	// if the camera was not found, add it
	if (tmp==NULL) {
	  AppendCamera(camera_ptr);
	}
      }
    }
  }
}

camera_t*
NewCamera(void) {
  camera_t* cam;

  cam=(camera_t*)malloc(sizeof(camera_t));
  cam->prefs.video1394_device=(char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_user = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_address = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename =(char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_path =(char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.v4l_dev_name =(char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.name = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.overlay_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  pthread_mutex_init(&cam->uimutex, NULL);
  return cam;

}

void
GetCameraData(int port, nodeid_t node, camera_t* cam) {

  cam->camera_info.handle=dc1394_create_handle(port);
  if (dc1394_get_camera_info(cam->camera_info.handle, node, &cam->camera_info)!=DC1394_SUCCESS)
    MainError("Could not get camera basic information!");
  if (dc1394_get_camera_misc_info(cam->camera_info.handle, cam->camera_info.id, &cam->misc_info)!=DC1394_SUCCESS)
    MainError("Could not get camera misc information!");
  if (dc1394_get_camera_feature_set(cam->camera_info.handle, cam->camera_info.id, &cam->feature_set)!=DC1394_SUCCESS)
    MainError("Could not get camera feature information!");
  GetFormat7Capabilities(cam);
  cam->image_pipe=NULL;
  pthread_mutex_lock(&cam->uimutex);
  cam->want_to_display=0;
  cam->bayer=NO_BAYER_DECODING;
  cam->stereo=NO_STEREO_DECODING;
  cam->bpp=8;
  CopyCameraPrefs(cam);
  pthread_mutex_unlock(&cam->uimutex);

}

void
AppendCamera(camera_t* cam) {

  camera_t* ptr;

  // if first camera:
  if (cameras==NULL) {
    cameras=cam;
    cam->prev=NULL;
    cam->next=NULL;
  }
  else {
    ptr=cameras; 
    while(ptr->next!=NULL) {
      ptr=ptr->next;
    }
    ptr->next=cam;
    cam->prev=ptr;
    cam->next=NULL;
  }
}

void
SetCurrentCamera(u_int64_t guid) {

  camera_t* ptr;
  ptr=cameras;

  while ((ptr->camera_info.euid_64!=guid)&&(ptr->next!=NULL)) {
    ptr=ptr->next;
  }
  if (ptr->camera_info.euid_64!=guid)
    fprintf(stderr,"Kaai! Can't find camera GUID in the camera stack!\n");
  else
    camera=ptr;
}

void
RemoveCamera(u_int64_t guid) {

  camera_t* ptr;
  ptr=cameras;
  while (ptr->camera_info.euid_64!=guid) {
    ptr=ptr->next;
  }

  V4lStopThread(ptr);
  FtpStopThread(ptr);
  SaveStopThread(ptr);
  DisplayStopThread(ptr);
  IsoStopThread(ptr);

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

  FreeCamera(ptr);
}

void
FreeCamera(camera_t* cam) {
  
  free(cam->prefs.video1394_device);
  free(cam->prefs.ftp_user);
  free(cam->prefs.ftp_address);
  free(cam->prefs.ftp_filename); 
  free(cam->prefs.ftp_path); 
  free(cam->prefs.v4l_dev_name); 
  free(cam->prefs.name);
  free(cam->prefs.save_filename);
  free(cam->prefs.overlay_filename);
  free(cam);
  cam=NULL;
}

