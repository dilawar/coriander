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
extern PrefsInfo preferences;
extern int silent_ui_update;

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
  if (dc1394_start_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not start ISO transmission");
  else {
    camera->misc_info.is_iso_on=DC1394_TRUE;
    UpdateIsoFrame();
  }
  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_stop_iso_transmission(camera->camera_info.handle, camera->camera_info.id)!=DC1394_SUCCESS)
    MainError("Could not stop ISO transmission");
  else {
    camera->misc_info.is_iso_on=DC1394_FALSE;
    UpdateIsoFrame();
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
  
  camera_ptr=(camera_t*)user_data;

  // close current display (we don't want display to be used by 2 threads at the same time 'cause SDL forbids it)
  DisplayStopThread(camera);

  // stop all FPS displays:
  StopFPSDisplay();

  // set current camera pointers:
  SetCurrentCamera(camera_ptr->camera_info.euid_64);

  if (camera->want_to_display>0)
    DisplayStartThread(camera);

  // redraw all:
  BuildAllWindows();
  UpdateAllWindows();

  // resume all FPS displays:
  ResumeFPSDisplay();
}

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data)
{
  int bpp;
  int state;

  IsoFlowCheck(&state);

  if (dc1394_set_format7_byte_per_packet(camera->camera_info.handle, camera->camera_info.id, 
					 camera->format7_info.edit_mode, (int)adj->value)!=DC1394_SUCCESS)
    MainError("Could not change Format7 bytes per packet");
  else
    camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].bpp=adj->value;

  dc1394_query_format7_byte_per_packet(camera->camera_info.handle, camera->camera_info.id,camera->format7_info.edit_mode,&bpp);

  IsoFlowResume(&state);
  //fprintf(stderr,"bpp: %d (should set to %d)\n",bpp, (int)adj->value);
}


void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data)
{
  camera->format7_info.edit_mode=(int)user_data;
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

  // if the mode is the 'live' mode:
  if (camera->format7_info.edit_mode==camera->misc_info.mode) {
    UpdateOptionFrame();
    UpdateFormat7BppRange();
    UpdateFormat7Ranges();
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
  }
}

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data)
{
  int sx, sy, px, py;

  sx=camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].size_x;
  sy=camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].size_y;
  px=camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].pos_x;
  py=camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].pos_y;

  switch((int)user_data)
    {
      case FORMAT7_SIZE_X:
	sx=adj->value;
	break;
      case FORMAT7_SIZE_Y:
	sy=adj->value;
	break;
      case FORMAT7_POS_X:
	px=adj->value;
	break;
      case FORMAT7_POS_Y:
	py=adj->value;
	break;
    }
  SetFormat7Crop(sx,sy,px,py);

  //fprintf(stderr,"Size: %d %d  Position: %d %d\n",info->size_x, info->size_y, info->pos_x, info->pos_y);
  // update bpp range here.
  UpdateFormat7BppRange();
}


void
on_preferences_window_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
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
      if (DisplayStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
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
  char tmp[STRING_SIZE];
  const char *camera_name_str =  "coriander/camera_names/";

  camera->name=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "camera_name_text")));
  sprintf(tmp,"%s%llx",camera_name_str, camera->camera_info.euid_64);
  
  gnome_config_set_string(tmp,camera->name);
  gnome_config_sync();
  BuildCameraMenu();
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
  preferences.display_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_display_period")));
  gnome_config_set_int("coriander/display/period",preferences.display_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_DISPLAY);
  if (service!=NULL) {
    info=service->data;
    info->period=preferences.display_period;
  }
}

void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  preferences.save_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_save_period")));
  gnome_config_set_int("coriander/save/period",preferences.save_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->period=preferences.save_period;
  }
}


void
on_prefs_v4l_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  v4lthread_info_t* info;
  chain_t* service;
  preferences.v4l_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_v4l_period")));
  gnome_config_set_int("coriander/v4l/period",preferences.v4l_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_V4L);
  if (service!=NULL) {
    info=service->data;
    info->period=preferences.v4l_period;
  }
}

void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  preferences.ftp_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_ftp_period")));
  gnome_config_set_int("coriander/ftp/period",preferences.ftp_period);
  gnome_config_sync();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->period=preferences.ftp_period;
  }
}


void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_address=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_address")));
  gnome_config_set_string("coriander/ftp/address",preferences.ftp_address);
  gnome_config_sync();
}


void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_path=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_path")));
  gnome_config_set_string("coriander/ftp/path",preferences.ftp_path);
  gnome_config_sync();

}


void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_user=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_user")));
  gnome_config_set_string("coriander/ftp/user",preferences.ftp_user);
  gnome_config_sync();

}


void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_password=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_password")));
  gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  gnome_config_sync();
}


void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_filename")));
  gnome_config_set_string("coriander/ftp/filename",preferences.ftp_filename);
  gnome_config_sync();
}


void
on_prefs_save_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.save_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_save_filename")));
  gnome_config_set_string("coriander/save/filename",preferences.save_filename);
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
    preferences.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    //fprintf(stderr,"setting save mode to sequential: %d\n",preferences.save_scratch);
    gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=preferences.save_scratch;
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
    preferences.save_scratch=SAVE_SCRATCH_OVERWRITE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    //fprintf(stderr,"setting save mode to overwrite: %d\n",preferences.save_scratch);
    gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=preferences.save_scratch;
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
    preferences.save_scratch=SAVE_SCRATCH_VIDEO;
    //fprintf(stderr,"setting save mode to video: %d\n",preferences.save_scratch);
    gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->scratch=preferences.save_scratch;
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
    preferences.save_convert=SAVE_CONVERT_ON;
  gnome_config_set_int("coriander/save/convert",preferences.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=preferences.save_convert;
  }
}


void
on_prefs_save_noconvert_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    preferences.save_convert=SAVE_CONVERT_OFF;
  gnome_config_set_int("coriander/save/convert",preferences.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=preferences.save_convert;
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
    preferences.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
    gnome_config_set_int("coriander/ftp/scratch",preferences.ftp_scratch);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
    service=GetService(camera,SERVICE_FTP);
    if (service!=NULL) {
      info=service->data;
      info->scratch=preferences.ftp_scratch;
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
    preferences.ftp_scratch=FTP_SCRATCH_OVERWRITE;
    gnome_config_set_int("coriander/ftp/scratch",preferences.ftp_scratch);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
    service=GetService(camera,SERVICE_FTP);
    if (service!=NULL) {
      info=service->data;
      info->scratch=preferences.ftp_scratch;
    }
  }
}


void
on_prefs_receive_method_activate      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.receive_method=(int)user_data;
  gnome_config_set_int("coriander/receive/method",preferences.receive_method);
  gnome_config_sync();
  UpdatePrefsReceiveFrame();
}


void
on_prefs_display_keep_ratio_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.display_keep_ratio=togglebutton->active;
  gnome_config_set_int("coriander/display/keep_ratio",preferences.display_keep_ratio);
  gnome_config_sync();
}


void
on_prefs_video1394_device_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.video1394_device=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_video1394_device")));
  gnome_config_set_string("coriander/receive/video1394_device",preferences.video1394_device);
  gnome_config_sync();

}

void
on_prefs_v4l_dev_name_changed      (GtkEditable     *editable,
                                  gpointer         user_data)
{
  preferences.v4l_dev_name=gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_v4l_dev_name")));
  gnome_config_set_string("coriander/v4l/v4l_dev_name",preferences.v4l_dev_name);
  gnome_config_sync();

}


void
on_prefs_receive_drop_frames_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.video1394_dropframes=togglebutton->active;
  gnome_config_set_int("coriander/receive/video1394_dropframes",preferences.video1394_dropframes);
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
  
  pthread_mutex_unlock(&camera->uimutex);
  camera->bayer_pattern=tmp;
  pthread_mutex_unlock(&camera->uimutex);
}

void
on_stereo_menu_activate               (GtkToggleButton *menuitem,
                                       gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  pthread_mutex_unlock(&camera->uimutex);
  camera->stereo=tmp;
  pthread_mutex_unlock(&camera->uimutex);

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
  pthread_mutex_lock(&camera->uimutex);
  camera->bpp=value;
  //fprintf(stderr,"uiinfo->bpp = %d\n",uiinfo->bpp);
  pthread_mutex_unlock(&camera->uimutex);
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
  camera_t* camera_ptr;
  camera_ptr=cameras;
  while (camera_ptr!=NULL) {
    //fprintf(stderr,"Trying to stop camera\n");
    if (camera_ptr->misc_info.is_iso_on==DC1394_TRUE) {
      if (dc1394_stop_iso_transmission(camera_ptr->camera_info.handle,camera_ptr->camera_info.id)!=DC1394_SUCCESS) {
	MainError("Could not stop ISO transmission");
      }
      else {
	//fprintf(stderr," ISO stopped\n");
	camera_ptr->misc_info.is_iso_on=DC1394_FALSE;
      } 
      if (camera_ptr==camera) {
	UpdateIsoFrame();
	UpdateTransferStatusFrame();
      } 
      usleep(50000);
    }
    camera_ptr=camera_ptr->next;
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
  camera_t* camera_ptr;
  camera_ptr=cameras;
  while (camera_ptr!=NULL) {
    //fprintf(stderr,"Trying to start camera\n");
    if (camera_ptr->misc_info.is_iso_on==DC1394_FALSE) {
      if (dc1394_start_iso_transmission(camera_ptr->camera_info.handle,camera_ptr->camera_info.id)!=DC1394_SUCCESS) {
	MainError("Could not stop ISO transmission");
      }
      else {
	//fprintf(stderr," ISO started\n");
	camera_ptr->misc_info.is_iso_on=DC1394_TRUE;
      } 
      if (camera_ptr==camera) {
	UpdateIsoFrame();
	UpdateTransferStatusFrame();
      } 
      usleep(50000);
    }
    camera_ptr=camera_ptr->next;
  }
}


void
on_prefs_save_date_tag_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    preferences.save_datenum=SAVE_TAG_DATE;
  gnome_config_set_int("coriander/save/datenum",preferences.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=preferences.save_datenum;
  }
}


void
on_prefs_save_num_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    preferences.save_datenum=SAVE_TAG_NUMBER;
  gnome_config_set_int("coriander/save/datenum",preferences.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=preferences.save_datenum;
  }
}


void
on_prefs_ftp_date_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    preferences.ftp_datenum=FTP_TAG_DATE;
  gnome_config_set_int("coriander/ftp/datenum",preferences.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->datenum=preferences.ftp_datenum;
  }
}


void
on_prefs_ftp_num_tag_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ftpthread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    preferences.ftp_datenum=FTP_TAG_NUMBER;
  gnome_config_set_int("coriander/ftp/datenum",preferences.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
  service=GetService(camera,SERVICE_FTP);
  if (service!=NULL) {
    info=service->data;
    info->datenum=preferences.ftp_datenum;
  }
}


void
on_ram_buffer_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.use_ram_buffer=togglebutton->active;
  gnome_config_set_int("coriander/save/use_ram_buffer",preferences.use_ram_buffer);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_ram_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ram_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"ram_buffer_size")));
  gnome_config_set_int("coriander/save/ram_buffer_size",preferences.ram_buffer_size);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_malloc_test_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  unsigned char *temp;
  char stemp[STRING_SIZE];

  // test if we can allocate enough memory
  sprintf(stemp,"Trying to allocate %d MB...", preferences.ram_buffer_size);
  MainStatus(stemp);
  temp=(unsigned char*)malloc(preferences.ram_buffer_size*1024*1024*sizeof(unsigned char));

  if (temp==NULL)
    MainStatus("\tFailed to allocate memory");
  else {
    MainStatus("\tAllocation succeeded");
    free(temp);
  }
}


void
on_dma_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.dma_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"dma_buffer_size")));
  gnome_config_set_int("coriander/receive/dma_buffer_size",preferences.dma_buffer_size);
  gnome_config_sync();
  //UpdatePrefsReceiveFrame();
}


void
on_display_redraw_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.display_redraw=DISPLAY_REDRAW_ON;
  else
    preferences.display_redraw=DISPLAY_REDRAW_OFF;
    
  gnome_config_set_int("coriander/display/redraw",preferences.display_redraw);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}


void
on_display_redraw_rate_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.display_redraw_rate=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(main_window,"display_redraw_rate")));
  gnome_config_set_float("coriander/display/redraw_rate",preferences.display_redraw_rate);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}

