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

#include "callbacks.h"

#define EEPROM_CNFG       0xF00U
#define TEST_CNFG         0xF04U
#define CCR_BASE          0xFFFFF0F00000ULL

extern GtkWidget *about_window;
extern GtkWidget *help_window;
extern GtkWidget *main_window;
extern GtkWidget *preferences_window;
extern camera_t* camera;
extern camera_t* cameras;
extern Prefs_t preferences;
extern int silent_ui_update;

#ifdef HAVE_SDLLIB
extern watchthread_info_t watchthread_info;
#endif

gboolean
on_main_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_exit(0);
  return FALSE;
}


void
on_file_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_exit(0);
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  about_window = create_about_window ();
  gtk_signal_connect(GTK_OBJECT(about_window), "realize",
		     GTK_SIGNAL_FUNC(window_set_icon), about_window);
  gtk_widget_show (about_window);
}


void
on_fps_activate                    (GtkMenuItem     *menuitem,
				    gpointer         user_data)
{
  int state;
  
  IsoFlowCheck(&state);
    
  if(dc1394_set_video_framerate(camera->camera_info.handle, camera->camera_info.id, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not set framerate");
  else
    camera->misc_info.framerate=(int)user_data;

  IsoFlowResume(&state);
}


void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_on(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not set camera 'on'");
}


void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_off(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not set camera 'off'");
}


void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_init_camera(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not initilize camera");
}

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (dc1394_set_trigger_polarity(camera->camera_info.handle, camera->camera_info.id,togglebutton->active)!=DC1394_SUCCESS)
    MainError("Cannot set trigger polarity");
  else
    camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_polarity=(int)togglebutton->active;
}


void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  if (dc1394_set_trigger_mode(camera->camera_info.handle, camera->camera_info.id, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not set trigger mode");
  else
    camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode=(int)user_data;
  UpdateTriggerFrame();
}

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, FEATURE_TRIGGER, togglebutton->active)!=DC1394_SUCCESS)
    MainError("Could not set external trigger source");
  else
    camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on=togglebutton->active;
  UpdateTriggerFrame();
}


void
on_memory_channel_activate              (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  camera->misc_info.save_channel=(int)user_data; // user data is an int.
  camera->misc_info.load_channel=(int)user_data; // user data is an int.
  UpdateMemoryFrame();
}



void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_memory_load(camera->camera_info.handle, camera->camera_info.id, camera->misc_info.load_channel)!=DC1394_SUCCESS)
    MainError("Cannot load memory channel");
  UpdateAllWindows();

}

void
on_save_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{ 
  unsigned long int timeout_bin=0;
  unsigned long int step;
  dc1394bool_t value=TRUE;
  step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);

  if (dc1394_set_memory_save_ch(camera->camera_info.handle, camera->camera_info.id, camera->misc_info.save_channel)!=DC1394_SUCCESS)
    MainError("Could not set memory save channel");
  else { 
    if (dc1394_memory_save(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
      MainError("Could not save setup to memory channel");
    else {
      while ((value==DC1394_TRUE) &&(timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
	usleep(step);
	if (dc1394_is_memory_save_in_operation(camera->camera_info.handle, camera->camera_info.id, &value)!=DC1394_SUCCESS)
	  MainError("Could not query if memory save is in operation");
	timeout_bin+=step;
      }
      if (timeout_bin>=(unsigned long int)(preferences.op_timeout*1000000.0))
	MainStatus("Save operation function timed-out!"); 
    }
  }
}

void
on_iso_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394bool_t status;

  if (dc1394_start_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not start ISO transmission");
  else {
    usleep(DELAY);
    if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id, &status)!=DC1394_SUCCESS)
      MainError("Could get ISO status");
    else {
      if (status==DC1394_FALSE) {
	MainError("ISO transmission refuses to start");
      }
      camera->misc_info.is_iso_on=status;
      UpdateIsoFrame();
    }
  }
  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394bool_t status;

  if (dc1394_stop_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not stop ISO transmission");
  else {
    usleep(DELAY);
    if (dc1394_get_iso_status(camera->camera_info.handle, camera->camera_info.id, &status)!=DC1394_SUCCESS)
      MainError("Could get ISO status");
    else {
      if (status==DC1394_TRUE) {
	MainError("ISO transmission refuses to stop");
      }
      camera->misc_info.is_iso_on=status;
      UpdateIsoFrame();
    }
  }
  UpdateTransferStatusFrame();
}


void
on_iso_restart_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  on_iso_stop_clicked(GTK_BUTTON(lookup_widget(main_window,"iso_stop")),NULL);
  on_iso_start_clicked(GTK_BUTTON(lookup_widget(main_window,"iso_start")),NULL);
  UpdateTransferStatusFrame();
}

void
on_camera_select_activate              (GtkMenuItem     *menuitem,
					gpointer         user_data)
{
  camera_t* camera_ptr;
  
  // close current display (we don't want display to be used by 2 threads at the same time 'cause SDL forbids it)
  DisplayStopThread(camera);

  camera_ptr=(camera_t*)user_data;

  // set current camera pointers:
  SetCurrentCamera(camera_ptr->camera_info.euid_64);

#ifdef HAVE_SDLLIB
  watchthread_info.draw=0;
  watchthread_info.mouse_down=0;
  watchthread_info.crop=0;
#endif

  if (camera->want_to_display>0)
    DisplayStartThread(camera);

  //fprintf(stderr,"camera: %s\n",camera->prefs.name);

  // redraw all:
  BuildAllWindows();
  UpdateAllWindows();

}

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data)
{ 
  int bpp;
  int state;
  int value;
  
  value=(int)adj->value;

  value=NearestValue(value,camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].min_bpp,
		     camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].min_bpp,
		     camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].max_bpp);

  // do something if we were called by a first generation signal:
  if (gtk_signal_n_emissions_by_name(GTK_OBJECT (adj), "changed")==0) {

    IsoFlowCheck(&state);
    
    if (dc1394_set_format7_byte_per_packet(camera->camera_info.handle, camera->camera_info.id, 
					   camera->format7_info.edit_mode, value)!=DC1394_SUCCESS)
      MainError("Could not change Format7 bytes per packet");

    if (dc1394_query_format7_byte_per_packet(camera->camera_info.handle, camera->camera_info.id,
					     camera->format7_info.edit_mode,&bpp)!=DC1394_SUCCESS) 
      MainError("Could not query Format7 bytes per packet");
    else {
      camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].bpp=bpp;
      if (bpp==0)
	fprintf(stderr,"BPP is zero in %s at line %d\n",__FUNCTION__,__LINE__);
      
      // tell the range to change its setting
      adj->value=bpp;
      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
      
      usleep(DELAY);
    }

    GetFormat7ModeInfo(camera, camera->format7_info.edit_mode);
    UpdateFormat7InfoFrame();
    IsoFlowResume(&state);
    
  }
} 

void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data)
{
  camera->format7_info.edit_mode=(int)user_data;
  GetFormat7ModeInfo(camera, camera->format7_info.edit_mode);
  UpdateFormat7Window();
}

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					    gpointer         user_data)
{
  int state;

  // if the mode is the 'live' mode:
  if (camera->format7_info.edit_mode==camera->misc_info.mode)
    IsoFlowCheck(&state);

  if (dc1394_set_format7_color_coding_id(camera->camera_info.handle, camera->camera_info.id, camera->format7_info.edit_mode, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not change Format7 color coding");
  else
    camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].color_coding_id=(int)user_data;

  GetFormat7ModeInfo(camera, camera->format7_info.edit_mode);
  UpdateOptionFrame();
  UpdateFormat7Window();
  /*UpdateFormat7BppRange();
    UpdateFormat7Ranges();
    UpdateFormat7InfoFrame();*/

  // if the mode is the 'live' mode:
  if (camera->format7_info.edit_mode==camera->misc_info.mode) {
    IsoFlowResume(&state);
  }

}


void
on_scale_value_changed             ( GtkAdjustment    *adj,
				     gpointer         user_data)
{
  switch((int)user_data) {
  case FEATURE_TEMPERATURE:
    if (dc1394_set_temperature(camera->camera_info.handle, camera->camera_info.id,adj->value)!=DC1394_SUCCESS)
      MainError("Could not set temperature");
    else
      camera->feature_set.feature[FEATURE_TEMPERATURE-FEATURE_MIN].target_value=adj->value;
    break;
  case FEATURE_WHITE_BALANCE+BU*4: // why oh why is there a *4?
    if (dc1394_set_white_balance(camera->camera_info.handle, camera->camera_info.id,adj->value, camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value)!=DC1394_SUCCESS)
      MainError("Could not set B/U white balance");
    else {
      camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value=adj->value;
      if (camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_BALANCE);
      }
    }
    break;
  case FEATURE_WHITE_BALANCE+RV*4: // why oh why is there a *4?
    if (dc1394_set_white_balance(camera->camera_info.handle, camera->camera_info.id, camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value, adj->value)!=DC1394_SUCCESS)
      MainError("Could not set R/V white balance");
    else {
      camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value=adj->value;
      if (camera->feature_set.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_BALANCE);
      }
    }
    break;
  default: // includes trigger_count
    if (dc1394_set_feature_value(camera->camera_info.handle, camera->camera_info.id,(int)user_data,adj->value)!=DC1394_SUCCESS)
      MainError("Could not set feature");
    else {
      camera->feature_set.feature[(int)user_data-FEATURE_MIN].value=adj->value;
      if ((int)user_data!=FEATURE_TRIGGER) {
	if (camera->feature_set.feature[(int)user_data-FEATURE_MIN].absolute_capable!=0) {
	  GetAbsValue((int)user_data);
	}
      }
    }
    // optical filter sometimes changes white balance (sony cameras) so we update the WB.
    if ((int)user_data==FEATURE_OPTICAL_FILTER) {
      UpdateRange(FEATURE_WHITE_BALANCE);
    }
    break;
  }
}

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data)
{
  int sx, sy, px, py;
  Format7ModeInfo_t* info;

  if (camera->format7_info.edit_mode>=0) { // check if F7 is supported
    info=&camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN];
    sx=info->size_x;
    sy=info->size_y;
    px=info->pos_x;
    py=info->pos_y;
    /*
    fprintf(stderr,"%d %d %d %d %d %d\n",info->max_size_x, info->max_size_y,
	    info->step_x, info->step_y, 
	    info->step_pos_x, info->step_pos_y);
    */
    switch((int)user_data) {
    case FORMAT7_SIZE_X:
      sx=adj->value;
      sx=NearestValue(sx,info->unit_size_x, info->unit_size_x, info->max_size_x - px);
      break;
    case FORMAT7_SIZE_Y:
      sy=adj->value;
      sy=NearestValue(sy,info->unit_size_y, info->unit_size_y, info->max_size_y - py);
      break;
    case FORMAT7_POS_X:
      px=adj->value;
      px=NearestValue(px,info->unit_pos_x, 0, info->max_size_x - info->unit_pos_x);
      break;
    case FORMAT7_POS_Y:
      py=adj->value;
      py=NearestValue(py,info->unit_pos_y, 0, info->max_size_y - info->unit_pos_y);
      break;
    }
    SetFormat7Crop(sx,sy,px,py, camera->format7_info.edit_mode);
    
    //fprintf(stderr,"Size: %d %d  Position: %d %d\n",info->size_x, info->size_y, info->pos_x, info->pos_y);
    // update bpp range here.
    GetFormat7ModeInfo(camera, camera->format7_info.edit_mode);
    UpdateFormat7BppRange();
    UpdateFormat7InfoFrame();
  }
}


void
on_preferences_window_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_signal_connect(GTK_OBJECT(preferences_window), "realize",
		     GTK_SIGNAL_FUNC(window_set_icon), preferences_window);
  gtk_widget_show(preferences_window);
}


void
on_service_iso_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (IsoStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      IsoStopThread(camera);
  }
  UpdatePrefsReceiveFrame();
}


void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      pthread_mutex_lock(&camera->uimutex);
      camera->want_to_display=1;
      pthread_mutex_unlock(&camera->uimutex);
      if (DisplayStartThread(camera)==-1) {
	gtk_toggle_button_set_active(togglebutton,0);
	camera->want_to_display=0;
      }
    } 
    else {
      DisplayStopThread(camera);
      pthread_mutex_lock(&camera->uimutex);
      camera->want_to_display=0;
      pthread_mutex_unlock(&camera->uimutex);
    } 
  }
  UpdatePrefsDisplayFrame();
}


void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (SaveStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      SaveStopThread(camera);
  }
  UpdatePrefsSaveFrame();
}


void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (FtpStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      FtpStopThread(camera);
  }
  UpdatePrefsFtpFrame();
}


void
on_service_v4l_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (V4lStartThread(camera)==-1) {
	gtk_toggle_button_set_active(togglebutton,0);
	gtk_widget_show(create_v4l_failure_window());
      }
    }
    else
      V4lStopThread(camera);
  }
  UpdatePrefsV4lFrame();
}


void
on_range_menu_activate             (GtkMenuItem     *menuitem,
				    gpointer         user_data)

{
  int feature;
  int action;

  // single auto variables:
  unsigned long int timeout_bin=0;
  unsigned long int step;
  dc1394bool_t value=TRUE;

  action=((int)user_data)%1000;
  feature=(((int)user_data)-action)/1000;

  switch (action) {
  case RANGE_MENU_OFF : // ============================== OFF ==============================
    if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, feature, FALSE)!=DC1394_SUCCESS)
      MainError("Could not set feature on/off");
    else {
      camera->feature_set.feature[feature-FEATURE_MIN].is_on=FALSE;
      UpdateRange(feature);
    }
    break;
  case RANGE_MENU_MAN : // ============================== MAN ==============================
      if (camera->feature_set.feature[feature-FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, feature, TRUE)!=DC1394_SUCCESS) {
	  MainError("Could not set feature on");
	  break;
	}
	else
	  camera->feature_set.feature[feature-FEATURE_MIN].is_on=TRUE;
      }
      if (dc1394_auto_on_off(camera->camera_info.handle, camera->camera_info.id, feature, FALSE)!=DC1394_SUCCESS)
	MainError("Could not set manual mode");
      else {
	camera->feature_set.feature[feature-FEATURE_MIN].auto_active=FALSE;
	if (camera->feature_set.feature[feature-FEATURE_MIN].absolute_capable)
	  SetAbsoluteControl(feature, FALSE);
	UpdateRange(feature);
      }
      break;
  case RANGE_MENU_AUTO : // ============================== AUTO ==============================
    if (camera->feature_set.feature[feature-FEATURE_MIN].on_off_capable) {
      if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, feature, TRUE)!=DC1394_SUCCESS) {
	MainError("Could not set feature on");
	break;
      }
      else
	camera->feature_set.feature[feature-FEATURE_MIN].is_on=TRUE;
    }
    if (dc1394_auto_on_off(camera->camera_info.handle, camera->camera_info.id, feature, TRUE)!=DC1394_SUCCESS)
      MainError("Could not set auto mode");
    else {
      camera->feature_set.feature[feature-FEATURE_MIN].auto_active=TRUE;
      if (camera->feature_set.feature[feature-FEATURE_MIN].absolute_capable)
	SetAbsoluteControl(feature, FALSE);
      UpdateRange(feature);
    }
    break;
    case RANGE_MENU_SINGLE : // ============================== SINGLE ==============================
      if (camera->feature_set.feature[feature-FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, feature, TRUE)!=DC1394_SUCCESS) {
	  MainError("Could not set feature on");
	  break;
	}
	else
	  camera->feature_set.feature[feature-FEATURE_MIN].is_on=TRUE;
      }
      step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);
      if (dc1394_start_one_push_operation(camera->camera_info.handle, camera->camera_info.id, feature)!=DC1394_SUCCESS)
	MainError("Could not start one-push operation");
      else {
	SetScaleSensitivity(GTK_WIDGET(menuitem),feature,FALSE);
	while ((value==DC1394_TRUE) && (timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
	  usleep(step);
	  if (dc1394_is_one_push_in_operation(camera->camera_info.handle, camera->camera_info.id, feature, &value)!=DC1394_SUCCESS)
	    MainError("Could not query one-push operation");
	  timeout_bin+=step;
	  UpdateRange(feature);
	}
	if (timeout_bin>=(unsigned long int)(preferences.op_timeout*1000000.0))
	  MainStatus("One-Push function timed-out!");

	if (camera->feature_set.feature[feature-FEATURE_MIN].absolute_capable)
	  SetAbsoluteControl(feature, FALSE);
	UpdateRange(feature);
	// should switch back to manual mode here. Maybe a recursive call??
	// >> Not necessary because UpdateRange reloads the status which folds
	// back to 'man' in the camera
      }
      break;
  case RANGE_MENU_ABSOLUTE : // ============================== ABSOLUTE ==============================
    if (camera->feature_set.feature[feature-FEATURE_MIN].on_off_capable) {
      if (dc1394_feature_on_off(camera->camera_info.handle, camera->camera_info.id, feature, TRUE)!=DC1394_SUCCESS) {
	MainError("Could not set feature on");
	break;
      }
      else
	camera->feature_set.feature[feature-FEATURE_MIN].is_on=TRUE;
    }
    SetAbsoluteControl(feature, TRUE);
    UpdateRange(feature);
    break;
  }
}


void
on_key_bindings_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  help_window = create_help_window();
  gtk_signal_connect(GTK_OBJECT(help_window), "realize",
		     GTK_SIGNAL_FUNC(window_set_icon), help_window);
  BuildHelpWindow();
  gtk_widget_show(help_window);
}

/******************************
 *       PREFERENCES          *
 ******************************/

void
on_camera_name_text_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp, *tmp_ptr;
  const char *camera_name_str =  "coriander/camera_names/";
  //fprintf(stderr,"name changed\n");
  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "camera_name_text")));
  sprintf(tmp,"%s%llx",camera_name_str, camera->camera_info.euid_64);
  gnome_config_set_string(tmp,tmp_ptr);
  gnome_config_sync();
  strcpy(camera->prefs.name,tmp_ptr);
  BuildCameraMenu();
  free(tmp);
}

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.op_timeout=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_op_timeout_scale")));
  gnome_config_set_float("coriander/global/one_push_timeout",preferences.op_timeout);
  gnome_config_sync();
}


void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.auto_update_frequency=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_update_scale")));
  gnome_config_set_float("coriander/global/auto_update_frequency",preferences.auto_update_frequency);
  gnome_config_sync();
}

void
on_prefs_display_period_changed        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  displaythread_info_t* info;
  chain_t* service;
  camera->prefs.display_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_display_period")));
  gnome_config_set_int("coriander/display/period",camera->prefs.display_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_DISPLAY);
  if (service!=NULL) {
    info=service->data;
    info->period=camera->prefs.display_period;
  }
}
/*
void
on_prefs_display_scale_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  displaythread_info_t* info;
  chain_t* service;
  preferences.display_scale=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_display_scale")));
  gnome_config_set_int("coriander/display/scale",preferences.display_scale);
  gnome_config_sync();
  service=GetService(camera,SERVICE_DISPLAY);
  if (service!=NULL) {
    info=service->data;
    info->scale_previous=info->scale;
    info->scale=preferences.display_scale;
  }
}
*/
void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  camera->prefs.save_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_save_period")));
  gnome_config_set_int("coriander/save/period",camera->prefs.save_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->period=camera->prefs.save_period;
  }
}


void
on_prefs_v4l_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  v4lthread_info_t* info;
  chain_t* service;
  camera->prefs.v4l_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_v4l_period")));
  gnome_config_set_int("coriander/v4l/period",camera->prefs.v4l_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_V4L);
  if (service!=NULL) {
    info=service->data;
    info->period=camera->prefs.v4l_period;
  }
}

void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  camera->prefs.ftp_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_ftp_period")));
  gnome_config_set_int("coriander/ftp/period",camera->prefs.ftp_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->period=camera->prefs.ftp_period;
  }
}


void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_address")));
  strcpy(camera->prefs.ftp_address,tmp_ptr);
  gnome_config_set_string("coriander/ftp/address",camera->prefs.ftp_address);
  gnome_config_sync();
}


void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_path")));
  strcpy(camera->prefs.ftp_path,tmp_ptr);
  gnome_config_set_string("coriander/ftp/path",camera->prefs.ftp_path);
  gnome_config_sync();

}


void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_user")));
  strcpy(camera->prefs.ftp_user,tmp_ptr);
  gnome_config_set_string("coriander/ftp/user",camera->prefs.ftp_user);
  gnome_config_sync();

}


void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_password")));
  strcpy(camera->prefs.ftp_password,tmp_ptr);
  // don't save passwords!
  //gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  //gnome_config_sync();
}


void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_filename")));
  strcpy(camera->prefs.ftp_filename,tmp_ptr);
  gnome_config_set_string("coriander/ftp/filename",camera->prefs.ftp_filename);
  gnome_config_sync();
}


void
on_prefs_save_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_save_filename")));
  strcpy(camera->prefs.save_filename,tmp_ptr);
  gnome_config_set_string("coriander/save/filename",camera->prefs.save_filename);
  gnome_config_sync();
}


void
on_prefs_update_power_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.auto_update=togglebutton->active;
  gnome_config_set_int("coriander/global/auto_update",preferences.auto_update);
  gnome_config_sync();
}

void
on_prefs_save_seq_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    gnome_config_set_int("coriander/save/scratch",camera->prefs.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=camera->prefs.save_scratch;
    }
  }
}


void
on_prefs_save_scratch_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_scratch=SAVE_SCRATCH_OVERWRITE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    gnome_config_set_int("coriander/save/scratch",camera->prefs.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=camera->prefs.save_scratch;
    }
  }
}


void
on_prefs_save_video_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_scratch=SAVE_SCRATCH_VIDEO;
    gnome_config_set_int("coriander/save/scratch",camera->prefs.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=camera->prefs.save_scratch;
    }
  }
}

void
on_prefs_save_convert_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_convert=SAVE_CONVERT_ON;
  gnome_config_set_int("coriander/save/convert",camera->prefs.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=camera->prefs.save_convert;
  }
}


void
on_prefs_save_noconvert_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_convert=SAVE_CONVERT_OFF;
  gnome_config_set_int("coriander/save/convert",camera->prefs.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=camera->prefs.save_convert;
  }
}


void
on_prefs_save_choose_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *dialog = create_get_filename_dialog();
  gtk_widget_show(dialog);
}

void
on_get_filename_dialog_ok_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
  gchar *filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog));
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_save_filename")),filename);
  gtk_widget_destroy(dialog);
}


void
on_get_filename_dialog_cancel_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(dialog);
}


void
on_prefs_ftp_seq_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
    gnome_config_set_int("coriander/ftp/scratch",camera->prefs.ftp_scratch);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
    service=GetService(camera,SERVICE_FTP);
    if (service!=NULL) {
      info=service->data;
      info->scratch=camera->prefs.ftp_scratch;
    }
  }
}


void
on_prefs_ftp_scratch_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.ftp_scratch=FTP_SCRATCH_OVERWRITE;
    gnome_config_set_int("coriander/ftp/scratch",camera->prefs.ftp_scratch);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
    service=GetService(camera,SERVICE_FTP);
    if (service!=NULL) {
      info=service->data;
      info->scratch=camera->prefs.ftp_scratch;
    }
  }
}


void
on_prefs_receive_method_activate      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.receive_method=(int)user_data;
  gnome_config_set_int("coriander/receive/method",camera->prefs.receive_method);
  gnome_config_sync();
  UpdatePrefsReceiveFrame();
}


void
on_prefs_display_keep_ratio_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.display_keep_ratio=togglebutton->active;
  gnome_config_set_int("coriander/display/keep_ratio",camera->prefs.display_keep_ratio);
  gnome_config_sync();
}


void
on_prefs_video1394_device_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_video1394_device")));
  strcpy(camera->prefs.video1394_device,tmp_ptr);
  gnome_config_set_string("coriander/receive/video1394_device",camera->prefs.video1394_device);
  gnome_config_sync();

}

void
on_prefs_v4l_dev_name_changed      (GtkEditable     *editable,
                                  gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_v4l_dev_name")));
  strcpy(camera->prefs.v4l_dev_name,tmp_ptr);
  gnome_config_set_string("coriander/v4l/v4l_dev_name",camera->prefs.v4l_dev_name);
  gnome_config_sync();

}


void
on_prefs_receive_drop_frames_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.video1394_dropframes=togglebutton->active;
  gnome_config_set_int("coriander/receive/video1394_dropframes",camera->prefs.video1394_dropframes);
  gnome_config_sync();
}


void
on_bayer_menu_activate           (GtkMenuItem     *menuitem,
				  gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  pthread_mutex_lock(&camera->uimutex);
  camera->bayer=tmp;
  pthread_mutex_unlock(&camera->uimutex);
  UpdateOptionFrame();
}

void
on_bayer_pattern_menu_activate           (GtkMenuItem     *menuitem,
					  gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  camera->bayer_pattern=tmp;
}

void
on_stereo_menu_activate               (GtkToggleButton *menuitem,
                                       gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  camera->stereo=tmp;

  UpdateOptionFrame();

}


void
on_trigger_count_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"trigger_count")));

  if (dc1394_set_feature_value(camera->camera_info.handle, camera->camera_info.id, FEATURE_TRIGGER, value)!=DC1394_SUCCESS)
    MainError("Could not set external trigger count");
  else
    camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].value=value;
}


void
on_mono16_bpp_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"mono16_bpp")));

  camera->bpp=value;
}


void
on_abs_entry_activate              (GtkEditable     *editable,
                                    gpointer         user_data)
{ 
  int feature;
  feature=(int)user_data;
  SetAbsValue(feature);
}


void
on_global_iso_stop_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394bool_t status;
  camera_t* camera_ptr;
  camera_ptr=cameras;

  if (preferences.sync_control==1) {
    if (dc1394_stop_iso_transmission(camera_ptr->camera_info.handle,63)!=DC1394_SUCCESS) {
      MainError("Could not perform broadcast ISO command");
    }
    usleep(DELAY);
    while (camera_ptr!=NULL) {
      if (dc1394_get_iso_status(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, &status)!=DC1394_SUCCESS)
	MainError("Could not get ISO status");
      else {
	if (status==DC1394_TRUE) 
	  MainError("Broacast ISO stop failed for a camera");
	else 
	  camera_ptr->misc_info.is_iso_on=DC1394_FALSE;
	if (camera_ptr==camera) {
	  UpdateIsoFrame();
	  UpdateTransferStatusFrame();
	} 
      }
      camera_ptr=camera_ptr->next;
    } 
  }
  else {
    while (camera_ptr!=NULL) {
      if (camera_ptr->misc_info.is_iso_on==DC1394_TRUE) {
	if (dc1394_stop_iso_transmission(camera_ptr->camera_info.handle,camera_ptr->camera_info.id)!=DC1394_SUCCESS) {
	  MainError("Could not stop ISO transmission");
	}
	else {
	  if (dc1394_get_iso_status(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, &status)!=DC1394_SUCCESS)
	    MainError("Could not get ISO status");
	  else {
	    if (status==DC1394_TRUE)
	      MainError("Broacast ISO stop failed for a camera");
	    else
	      camera_ptr->misc_info.is_iso_on=DC1394_FALSE;
	  }
	} 
	if (camera_ptr==camera) {
	  UpdateIsoFrame();
	  UpdateTransferStatusFrame();
	} 
      }
      camera_ptr=camera_ptr->next;
    } 
  }
}


void
on_global_iso_restart_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  on_global_iso_stop_clicked(GTK_BUTTON(lookup_widget(main_window,"global_iso_stop")),NULL);
  on_global_iso_start_clicked(GTK_BUTTON(lookup_widget(main_window,"global_iso_start")),NULL);
}


void
on_global_iso_start_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{  
  dc1394bool_t status;
  camera_t* camera_ptr;
  camera_ptr=cameras;

  if (preferences.sync_control==1) {
    if (dc1394_start_iso_transmission(camera_ptr->camera_info.handle,63)!=DC1394_SUCCESS) {
      MainError("Could not perform broadcast ISO command");
    }
    usleep(DELAY);
    while (camera_ptr!=NULL) {
      if (dc1394_get_iso_status(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, &status)!=DC1394_SUCCESS)
	MainError("Could not get ISO status");
      else {
	if (status==DC1394_FALSE) 
	  MainError("Broacast ISO start failed for a camera");
	else 
	  camera_ptr->misc_info.is_iso_on=DC1394_TRUE;
	if (camera_ptr==camera) {
	  UpdateIsoFrame();
	  UpdateTransferStatusFrame();
	} 
      }
      camera_ptr=camera_ptr->next;
    } 
  }
  else { // no sync:
    while (camera_ptr!=NULL) {
      if (camera_ptr->misc_info.is_iso_on==DC1394_FALSE) {
	if (dc1394_start_iso_transmission(camera_ptr->camera_info.handle,camera_ptr->camera_info.id)!=DC1394_SUCCESS) {
	  MainError("Could not stop ISO transmission");
	}
	else {
	  if (dc1394_get_iso_status(camera_ptr->camera_info.handle, camera_ptr->camera_info.id, &status)!=DC1394_SUCCESS)
	    MainError("Could not get ISO status");
	  else {
	    if (status==DC1394_FALSE)
	      MainError("Broacast ISO stop failed for a camera");
	    else
	      camera_ptr->misc_info.is_iso_on=DC1394_TRUE;
	  }
	} 
	if (camera_ptr==camera) {
	  UpdateIsoFrame();
	  UpdateTransferStatusFrame();
	} 
      }
      camera_ptr=camera_ptr->next;
    } 
  }
}


void
on_prefs_save_date_tag_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_datenum=SAVE_TAG_DATE;
  gnome_config_set_int("coriander/save/datenum",camera->prefs.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.save_datenum;
  }
}


void
on_prefs_save_num_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_datenum=SAVE_TAG_NUMBER;
  gnome_config_set_int("coriander/save/datenum",camera->prefs.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.save_datenum;
  }
}


void
on_prefs_ftp_date_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.ftp_datenum=FTP_TAG_DATE;
  gnome_config_set_int("coriander/ftp/datenum",camera->prefs.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.ftp_datenum;
  }
}


void
on_prefs_ftp_num_tag_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.ftp_datenum=FTP_TAG_NUMBER;
  gnome_config_set_int("coriander/ftp/datenum",camera->prefs.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.ftp_datenum;
  }
}


void
on_ram_buffer_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.use_ram_buffer=togglebutton->active;
  gnome_config_set_int("coriander/save/use_ram_buffer",camera->prefs.use_ram_buffer);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_ram_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.ram_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"ram_buffer_size")));
  gnome_config_set_int("coriander/save/ram_buffer_size",camera->prefs.ram_buffer_size);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_malloc_test_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  unsigned char *temp;
  char *stemp;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

  // test if we can allocate enough memory
  sprintf(stemp,"Trying to allocate %d MB...", camera->prefs.ram_buffer_size);
  MainStatus(stemp);
  temp=(unsigned char*)malloc(camera->prefs.ram_buffer_size*1024*1024*sizeof(unsigned char));

  if (temp==NULL)
    MainStatus("\tFailed to allocate memory");
  else {
    MainStatus("\tAllocation succeeded");
    free(temp);
  }

  free(stemp);
}


void
on_dma_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.dma_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"dma_buffer_size")));
  gnome_config_set_int("coriander/receive/dma_buffer_size",camera->prefs.dma_buffer_size);
  gnome_config_sync();
}


void
on_display_redraw_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    camera->prefs.display_redraw=DISPLAY_REDRAW_ON;
  else
    camera->prefs.display_redraw=DISPLAY_REDRAW_OFF;
    
  gnome_config_set_int("coriander/display/redraw",camera->prefs.display_redraw);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}


void
on_display_redraw_rate_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.display_redraw_rate=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(main_window,"display_redraw_rate")));
  gnome_config_set_float("coriander/display/redraw_rate",camera->prefs.display_redraw_rate);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}


void
on_sync_control_button_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.sync_control=togglebutton->active;
  gnome_config_set_int("coriander/global/sync_control",preferences.sync_control);
  gnome_config_sync();
}


