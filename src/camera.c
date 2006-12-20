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

int
GetCameraNodes(void) {

  int err;
  dc1394camera_t **dccameras=NULL;
  unsigned int camnum=0;
  camera_t* camera_ptr;
  int i;
  //fprintf(stderr,"found %d cameras\n",camnum);

  // find the cameras using classic libdc functions:
  err=dc1394_find_cameras(&dccameras,&camnum);

  //fprintf(stderr,"found %d cameras\n",camnum);

  /*
  fprintf(stderr,"error code: %d-%d\n",err,DC1394_NO_CAMERA);
  
  if (err!=DC1394_NO_CAMERA)
    DC1394_ERR_CHK(err,"Error getting the cameras");
  */
  // create a list of cameras with coriander's camera type camera_t
  for (i=0;i<camnum;i++) {
    camera_ptr=NewCamera();
    // copy the info in the dc structure into the coriander struct.
    camera_ptr->camera_info=dccameras[i];

    //fprintf(stderr,"0x%llx - 0x%llx\n",dccameras[i]->euid_64,camera_ptr->camera_info->euid_64);

    //fprintf(stderr,"Getting camera data\n");
    GetCameraData(camera_ptr);
    //fprintf(stderr,"Adding camera\n");

    AppendCamera(camera_ptr);
  }

  // free the temp dccameras:
  free(dccameras);

  //fprintf(stderr,"Done getting nodes\n");

  return err;;
}

camera_t*
NewCamera(void) {
  camera_t* cam;

  cam=(camera_t*)malloc(sizeof(camera_t));
  cam->prefs.ftp_user = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_address = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_password = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_path = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.v4l_dev_name = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.name = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename_ext = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename_base = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename_ext = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename_base = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.overlay_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->bayer_pattern=DC1394_COLOR_FILTER_RGGB;
  pthread_mutex_init(&cam->uimutex, NULL);
  //fprintf(stderr,"new camera allocated\n");
  return cam;

}

void
GetCameraData(camera_t* cam) {

  if (cam->camera_info->bmode_capable>0) {
    // set b-mode and reprobe modes,... (higher fps formats might not be reported as available in legacy mode)
    dc1394_video_set_operation_mode(cam->camera_info, DC1394_OPERATION_MODE_1394B);
  }
  if (dc1394_get_camera_feature_set(cam->camera_info, &cam->feature_set)!=DC1394_SUCCESS)
    Error("Could not get camera feature information!");

  //fprintf(stderr,"Grabbing F7 stuff\n");
  GetFormat7Capabilities(cam);
  cam->image_pipe=NULL;
  pthread_mutex_lock(&cam->uimutex);
  cam->want_to_display=0;
  cam->bayer=-1;
  //cam->bayer_pattern=DC1394_COLOR_FILTER_RGGB
  cam->stereo=-1;
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

  while ((ptr->camera_info->euid_64!=guid)&&(ptr->next!=NULL)) {
    ptr=ptr->next;
  }
  if (ptr->camera_info->euid_64!=guid)
    fprintf(stderr,"Kaai! Can't find camera GUID in the camera stack!\n");
  else
    camera=ptr;
}

void
RemoveCamera(u_int64_t guid) {

  camera_t* ptr;
  ptr=cameras;
  while (ptr->camera_info->euid_64!=guid) {
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
  ptr=NULL;
}

void
FreeCamera(camera_t* cam)
{  
  free(cam->prefs.ftp_user);
  free(cam->prefs.ftp_address);
  free(cam->prefs.ftp_filename); 
  free(cam->prefs.ftp_password); 
  free(cam->prefs.ftp_path); 
  free(cam->prefs.v4l_dev_name); 
  free(cam->prefs.name);
  free(cam->prefs.save_filename);
  free(cam->prefs.save_filename_ext);
  free(cam->prefs.save_filename_base);
  free(cam->prefs.overlay_filename);
  free(cam->camera_info);
  free(cam);
}

