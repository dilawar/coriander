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

extern GtkWidget *format7_window;
extern GtkWidget *about_window;
extern GtkWidget *absolute_settings_window;
extern GtkWidget *help_window;
extern GtkWidget *commander_window;
extern GtkWidget *preferences_window;
extern dc1394_camerainfo *camera;
extern dc1394_feature_set *feature_set;
extern dc1394_camerainfo *cameras;
extern dc1394_feature_set *feature_sets;
extern dc1394_miscinfo *misc_info;
extern dc1394_miscinfo *misc_infos;
extern dc1394_cameracapture *capture;
extern dc1394_cameracapture *captures;
extern Format7Info *format7_info;
extern Format7Info *format7_infos;
extern uiinfo_t *uiinfo;
extern uiinfo_t *uiinfos;
extern int current_camera;
extern PrefsInfo preferences; 
extern int silent_ui_update;
extern char* feature_scale_list[NUM_FEATURES];
extern int camera_num;
gboolean
on_commander_window_delete_event       (GtkWidget       *widget,
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
    
  if(dc1394_set_video_framerate(camera->handle, camera->id, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not set framerate");
  else
    misc_info->framerate=(int)user_data;

  IsoFlowResume(&state);
}


void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_on(camera->handle, camera->id)!=DC1394_SUCCESS)
    MainError("Could not set camera 'on'");
}


void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_off(camera->handle, camera->id)!=DC1394_SUCCESS)
    MainError("Could not set camera 'off'");
}


void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_init_camera(camera->handle, camera->id)!=DC1394_SUCCESS)
    MainError("Could not initilize camera");
}

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (dc1394_set_trigger_polarity(camera->handle,camera->id,togglebutton->active)!=DC1394_SUCCESS)
    MainError("Cannot set trigger polarity");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_polarity=(int)togglebutton->active;
}


void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  if (dc1394_set_trigger_mode(camera->handle, camera->id, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not set trigger mode");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode=(int)user_data;
  UpdateTriggerFrame();
}

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (dc1394_feature_on_off(camera->handle, camera->id, FEATURE_TRIGGER, togglebutton->active)!=DC1394_SUCCESS)
    MainError("Could not set external trigger source");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on=togglebutton->active;
  UpdateTriggerFrame();
}


void
on_memory_channel_activate              (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  misc_info->save_channel=(int)user_data; // user data is an int.
  misc_info->load_channel=(int)user_data; // user data is an int.
  UpdateMemoryFrame();
}



void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_memory_load(camera->handle,camera->id, misc_info->load_channel)!=DC1394_SUCCESS)
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

  if (dc1394_set_memory_save_ch(camera->handle,camera->id, misc_info->save_channel)!=DC1394_SUCCESS)
    MainError("Could not set memory save channel");
  else { 
    if (dc1394_memory_save(camera->handle,camera->id)!=DC1394_SUCCESS)
      MainError("Could not save setup to memory channel");
    else {
      while ((value==DC1394_TRUE) &&(timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
	usleep(step);
	if (dc1394_is_memory_save_in_operation(camera->handle,camera->id, &value)!=DC1394_SUCCESS)
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
  if (dc1394_start_iso_transmission(camera->handle,camera->id)!=DC1394_SUCCESS)
    MainError("Could not start ISO transmission");
  else {
    misc_info->is_iso_on=DC1394_TRUE;
    UpdateIsoFrame();
  }
  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_stop_iso_transmission(camera->handle,camera->id)!=DC1394_SUCCESS)
    MainError("Could not stop ISO transmission");
  else {
    misc_info->is_iso_on=DC1394_FALSE;
    UpdateIsoFrame();
  }
  UpdateTransferStatusFrame();
}


void
on_iso_restart_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  on_iso_stop_clicked(GTK_BUTTON(lookup_widget(commander_window,"iso_stop")),NULL);
  on_iso_start_clicked(GTK_BUTTON(lookup_widget(commander_window,"iso_start")),NULL);
  UpdateTransferStatusFrame();
}

void
on_camera_select_activate              (GtkMenuItem     *menuitem,
					gpointer         user_data)
{
 
  // close current display (we don't want display to be used by 2 threads at the same time 'cause SDL forbids it)
  DisplayStopThread(current_camera);

  // stop all FPS displays:
  StopFPSDisplay(current_camera);

  // set current camera pointers:
  SelectCamera((int)user_data);

  if (uiinfo->want_to_display>0)
    DisplayStartThread();

  // redraw all:
  BuildAllWindows();
  UpdateAllWindows();

  // resume all FPS displays:
  ResumeFPSDisplay(current_camera);
}

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data)
{
  int bpp;
  int state;

  IsoFlowCheck(&state);

  if (dc1394_set_format7_byte_per_packet(camera->handle, camera->id, 
					 format7_info->edit_mode, (int)adj->value)!=DC1394_SUCCESS)
    MainError("Could not change Format7 bytes per packet");
  else
    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].bpp=adj->value;

  dc1394_query_format7_byte_per_packet(camera->handle,camera->id,format7_info->edit_mode,&bpp);

  IsoFlowResume(&state);
  //fprintf(stderr,"bpp: %d (should set to %d)\n",bpp, (int)adj->value);
}


void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data)
{
  format7_info->edit_mode=(int)user_data;
  UpdateFormat7Window();
}

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					    gpointer         user_data)
{
  int state;

  // if the mode is the 'live' mode:
  if (format7_info->edit_mode==misc_info->mode)
    IsoFlowCheck(&state);

  if (dc1394_set_format7_color_coding_id(camera->handle, camera->id, format7_info->edit_mode, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not change Format7 color coding");
  else
    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].color_coding_id=(int)user_data;

  // if the mode is the 'live' mode:
  if (format7_info->edit_mode==misc_info->mode) {
    UpdateOptionFrame();
    UpdateFormat7BppRange();
    UpdateFormat7Ranges();
    IsoFlowResume(&state);
  }

}


void
on_format7_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (format7_window);
}

void
on_scale_value_changed             ( GtkAdjustment    *adj,
				     gpointer         user_data)
{
  switch((int)user_data) {
  case FEATURE_TEMPERATURE:
    if (dc1394_set_temperature(camera->handle,camera->id,adj->value)!=DC1394_SUCCESS)
      MainError("Could not set temperature");
    else
      feature_set->feature[FEATURE_TEMPERATURE-FEATURE_MIN].target_value=adj->value;
    break;
  case FEATURE_WHITE_BALANCE+BU*4: // why oh why is there a *4?
    if (dc1394_set_white_balance(camera->handle,camera->id,adj->value, feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value)!=DC1394_SUCCESS)
      MainError("Could not set B/U white balance");
    else {
      feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value=adj->value;
      if (feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_BALANCE);
      }
    }
    break;
  case FEATURE_WHITE_BALANCE+RV*4: // why oh why is there a *4?
    if (dc1394_set_white_balance(camera->handle,camera->id, feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value, adj->value)!=DC1394_SUCCESS)
      MainError("Could not set R/V white balance");
    else {
      feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value=adj->value;
      if (feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_BALANCE);
      }
    }
    break;
  default: // includes trigger_count
    if (dc1394_set_feature_value(camera->handle,camera->id,(int)user_data,adj->value)!=DC1394_SUCCESS)
      MainError("Could not set feature");
    else {
      feature_set->feature[(int)user_data-FEATURE_MIN].value=adj->value;
      if ((int)user_data!=FEATURE_TRIGGER) {
	if (feature_set->feature[(int)user_data-FEATURE_MIN].absolute_capable!=0) {
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
  GtkAdjustment* adj2;
  int state;
  int step;
  Format7ModeInfo *info;

  info=&(format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN]);

  switch((int)user_data)
    {
      case FORMAT7_SIZE_X:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(&state);
	step=info->step_x;
	adj->value=(((int)adj->value)/step)*step;
	if (dc1394_set_format7_image_size(camera->handle, camera->id, format7_info->edit_mode, adj->value, info->size_y)!=DC1394_SUCCESS)
	  MainError("Could not set Format7 image size");
	else  {
	  info->size_x=adj->value;
	  // adjust the pos_x adjustment so that (size_x+pos_x)<=max_size_x:
	  adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hposition_scale")));
	  adj2->upper=info->max_size_x-adj->value;
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	}
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(&state);
	break;

      case FORMAT7_SIZE_Y:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(&state);
	step=info->step_y;
	adj->value=(((int)adj->value)/step)*step;
	if (dc1394_set_format7_image_size(camera->handle,camera->id, format7_info->edit_mode, info->size_x, adj->value)!=DC1394_SUCCESS)
	  MainError("Could not set Format7 image size");
	else {
	  info->size_y=adj->value;
	  // adjust the pos_y adjustment so that (size_y+pos_y)<=max_size_y:
	  adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vposition_scale")));
	  adj2->upper=info->max_size_y-adj->value;
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	}
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(&state);
	break;
	
      case FORMAT7_POS_X:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(&state);
	if (info->use_unit_pos>0)
	  step=info->step_pos_x;
	else
	  step=info->step_x;
	adj->value=(((int)adj->value)/step)*step;
	if (dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode, adj->value, info->pos_y)!=DC1394_SUCCESS)
	  MainError("Could not set Format7 image position");
	else {
	  info->pos_x=adj->value;
	  // adjust the size_x adjustment so that (size_x+pos_x)<=max_size_x:
	  adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hsize_scale")));
	  adj2->upper=info->max_size_x-adj->value;
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	}
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(&state);
	break;

      case FORMAT7_POS_Y:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(&state);
	if (info->use_unit_pos>0)
	  step=info->step_pos_y;
	else
	  step=info->step_y;
	adj->value=(((int)adj->value)/step)*step;
	if (dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode, info->pos_x, adj->value)!=DC1394_SUCCESS)
	  MainError("Cannot set Format7 image position");
	else {
	  info->pos_y=adj->value;
	  // adjust the size_y adjustment so that (size_y+pos_y)<=max_size_y:
	  adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vsize_scale")));
	  adj2->upper=info->max_size_y-adj->value;
	  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	  gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	}
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(&state);
	break;

      default:
	MainError("Bad FORMAT7 scale ID passed to 'on_format7_value_changed'\n");
	break;
    }
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
      if (IsoStartThread()==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      IsoStopThread();
    //CleanThreads(CLEAN_MODE_UI_UPDATE_NOT_ISO);
  }

}


void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      //if (GetService(SERVICE_ISO,current_camera)==NULL)
      //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
      pthread_mutex_lock(&uiinfo->mutex);
      uiinfo->want_to_display=1;
      pthread_mutex_unlock(&uiinfo->mutex);
      if (DisplayStartThread()==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    } 
    else {
      DisplayStopThread(current_camera);
      pthread_mutex_lock(&uiinfo->mutex);
      uiinfo->want_to_display=0;
      pthread_mutex_unlock(&uiinfo->mutex);
    } 
  }
}


void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      //if (GetService(SERVICE_ISO,current_camera)==NULL)
      //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
      if (SaveStartThread()==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      SaveStopThread();
  }
}


void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      //if (GetService(SERVICE_ISO,current_camera)==NULL)
      //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
      if (FtpStartThread()==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      FtpStopThread();
  }
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
    if (dc1394_feature_on_off(camera->handle, camera->id, feature, FALSE)!=DC1394_SUCCESS)
      MainError("Could not set feature on/off");
    else {
      feature_set->feature[feature-FEATURE_MIN].is_on=FALSE;
      UpdateRange(commander_window,feature);
    }
    break;
  case RANGE_MENU_MAN : // ============================== MAN ==============================
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE)!=DC1394_SUCCESS) {
	  MainError("Could not set feature on");
	  break;
	}
	else
	  feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
      }
      if (dc1394_auto_on_off(camera->handle, camera->id, feature, FALSE)!=DC1394_SUCCESS)
	MainError("Could not set manual mode");
      else {
	feature_set->feature[feature-FEATURE_MIN].auto_active=FALSE;
	UpdateRange(commander_window,feature);
      }
      break;
  case RANGE_MENU_AUTO : // ============================== AUTO ==============================
    if (feature_set->feature[feature-FEATURE_MIN].on_off_capable) {
      if (dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE)!=DC1394_SUCCESS) {
	MainError("Could not set feature on");
	break;
      }
      else
	feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
    }
    if (dc1394_auto_on_off(camera->handle, camera->id, feature, TRUE)!=DC1394_SUCCESS)
      MainError("Could not set auto mode");
    else {
      feature_set->feature[feature-FEATURE_MIN].auto_active=TRUE;
      UpdateRange(commander_window,feature);
    }
    break;
    case RANGE_MENU_SINGLE : // ============================== SINGLE ==============================
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE)!=DC1394_SUCCESS) {
	  MainError("Could not set feature on");
	  break;
	}
	else
	  feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
      }
      step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);
      if (dc1394_start_one_push_operation(camera->handle, camera->id, feature)!=DC1394_SUCCESS)
	MainError("Could not start one-push operation");
      else {
	SetScaleSensitivity(GTK_WIDGET(menuitem),feature,FALSE);
	while ((value==DC1394_TRUE) && (timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
	  usleep(step);
	  if (dc1394_is_one_push_in_operation(camera->handle, camera->id, feature, &value)!=DC1394_SUCCESS)
	    MainError("Could not query one-push operation");
	  timeout_bin+=step;
	  UpdateRange(commander_window,feature);
	}
	if (timeout_bin>=(unsigned long int)(preferences.op_timeout*1000000.0))
	  MainStatus("One-Push function timed-out!");
	UpdateRange(commander_window,feature);
	// should switch back to manual mode here. Maybe a recursive call??
	// >> Not necessary because UpdateRange reloads the status which folds
	// back to 'man' in the camera
      }
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

  preferences.camera_names[current_camera]=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window, "camera_name_text")));
  sprintf(tmp,"%s%llx",camera_name_str, camera->euid_64);
  
  gnome_config_set_string(tmp,preferences.camera_names[current_camera]);
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
  preferences.display_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(commander_window,"prefs_display_period")));
  gnome_config_set_int("coriander/display/period",preferences.display_period);
  gnome_config_sync();
}

void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.save_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(commander_window,"prefs_save_period")));
  gnome_config_set_int("coriander/save/period",preferences.save_period);
  gnome_config_sync();
}


void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(commander_window,"prefs_ftp_period")));
  gnome_config_set_int("coriander/ftp/period",preferences.ftp_period);
  gnome_config_sync();
}


void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_address=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_ftp_address")));
  gnome_config_set_string("coriander/ftp/address",preferences.ftp_address);
  gnome_config_sync();
}


void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_path=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_ftp_path")));
  gnome_config_set_string("coriander/ftp/path",preferences.ftp_path);
  gnome_config_sync();

}


void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_user=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_ftp_user")));
  gnome_config_set_string("coriander/ftp/user",preferences.ftp_user);
  gnome_config_sync();

}


void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_password=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_ftp_password")));
  gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  gnome_config_sync();
}


void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_ftp_filename")));
  gnome_config_set_string("coriander/ftp/filename",preferences.ftp_filename);
  gnome_config_sync();
}


void
on_prefs_save_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.save_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_save_filename")));
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
  if (togglebutton->active)
    preferences.save_scratch=SAVE_SCRATCH_SEQUENTIAL;
  gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_prefs_save_scratch_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.save_scratch=SAVE_SCRATCH_OVERWRITE;
  gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_prefs_save_video_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.save_scratch=SAVE_SCRATCH_SEQUENCE;
  gnome_config_set_int("coriander/save/scratch",preferences.save_scratch);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}

void
on_prefs_save_convert_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.save_convert=SAVE_CONVERT_ON;
  gnome_config_set_int("coriander/save/convert",preferences.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_prefs_save_noconvert_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.save_convert=SAVE_CONVERT_OFF;
  gnome_config_set_int("coriander/save/convert",preferences.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
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
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_save_filename")),filename);
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
  if (togglebutton->active)
    preferences.ftp_scratch=FTP_SCRATCH_SEQUENTIAL;
  gnome_config_set_int("coriander/ftp/scratch",preferences.ftp_scratch);
  gnome_config_sync();
}


void
on_prefs_ftp_scratch_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.ftp_scratch=FTP_SCRATCH_OVERWRITE;
  gnome_config_set_int("coriander/ftp/scratch",preferences.ftp_scratch);
  gnome_config_sync();
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
  preferences.video1394_device=gtk_entry_get_text(GTK_ENTRY(lookup_widget(commander_window,"prefs_video1394_device")));
  gnome_config_set_string("coriander/receive/video1394_device",preferences.video1394_device);
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
  
  pthread_mutex_lock(&uiinfo->mutex);
  uiinfo->bayer=tmp;
  pthread_mutex_unlock(&uiinfo->mutex);
  UpdateOptionFrame();
}

void
on_bayer_pattern_menu_activate           (GtkMenuItem     *menuitem,
					  gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  pthread_mutex_unlock(&uiinfo->mutex);
  uiinfo->bayer_pattern=tmp;
  pthread_mutex_unlock(&uiinfo->mutex);
}

void
on_stereo_menu_activate               (GtkToggleButton *menuitem,
                                       gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  pthread_mutex_unlock(&uiinfo->mutex);
  uiinfo->stereo=tmp;
  pthread_mutex_unlock(&uiinfo->mutex);

  UpdateOptionFrame();

}


void
on_trigger_count_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(commander_window,"trigger_count")));

  if (dc1394_set_feature_value(camera->handle, camera->id, FEATURE_TRIGGER, value)!=DC1394_SUCCESS)
    MainError("Could not set external trigger count");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].value=value;
}


void
on_mono16_bpp_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(commander_window,"mono16_bpp")));
  pthread_mutex_lock(&uiinfo->mutex);
  uiinfo->bpp=value;
  //fprintf(stderr,"uiinfo->bpp = %d\n",uiinfo->bpp);
  pthread_mutex_unlock(&uiinfo->mutex);
}


void
on_absolute_settings_window_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show(absolute_settings_window);
}


void
on_abs_autoexp_switch_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_EXPOSURE, togglebutton->active);
}


void
on_abs_iris_switch_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_IRIS, togglebutton->active);
}


void
on_abs_shutter_switch_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_SHUTTER, togglebutton->active);
}


void
on_abs_gain_switch_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_GAIN, togglebutton->active);
}


void
on_abs_hue_switch_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_HUE, togglebutton->active);
}


void
on_abs_saturation_switch_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_SATURATION, togglebutton->active);
}


void
on_abs_whitebal_switch_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_WHITE_BALANCE, togglebutton->active);
}


void
on_abs_brightness_switch_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_BRIGHTNESS, togglebutton->active);
}


void
on_abs_zoom_switch_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_ZOOM, togglebutton->active);
}


void
on_abs_focus_switch_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_FOCUS, togglebutton->active);
}


void
on_abs_tilt_switch_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_TILT, togglebutton->active);
}


void
on_abs_pan_switch_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  SetAbsoluteControl(FEATURE_PAN, togglebutton->active);
}


void
on_abs_zoom_entry_activate              (GtkEditable     *editable,
                                        gpointer         user_data)
{ 
  SetAbsValue(FEATURE_ZOOM);
}


void
on_abs_focus_entry_activate             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_FOCUS);
}


void
on_abs_tilt_entry_activate              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_TILT);
}


void
on_abs_pan_entry_activate               (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_PAN);
}


void
on_abs_whitebal_entry_activate          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_WHITE_BALANCE);
}


void
on_abs_saturation_entry_activate        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_SATURATION);
}


void
on_abs_hue_entry_activate               (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_HUE);
}

void
on_abs_autoexp_entry_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_EXPOSURE);
}


void
on_abs_iris_entry_activate              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_IRIS);
}


void
on_abs_shutter_entry_activate           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_SHUTTER);
}


void
on_abs_gain_entry_activate              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_GAIN);
}


void
on_abs_brightness_entry_activate        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  SetAbsValue(FEATURE_BRIGHTNESS);
}

void
on_global_iso_stop_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  int i;
  for (i=0;i<camera_num;i++) {
    //fprintf(stderr,"Trying to stop camera %d\n",i);
    if (misc_infos[i].is_iso_on!=DC1394_FALSE) {
      if (dc1394_stop_iso_transmission(camera[i].handle,camera[i].id)!=DC1394_SUCCESS) {
	MainError("Could not stop ISO transmission");
      }
      else {
	//fprintf(stderr," ISO stopped for camera %d\n",i);
	misc_info[i].is_iso_on=DC1394_FALSE;
      }
      if (i==current_camera) {
	UpdateIsoFrame();
	UpdateTransferStatusFrame();
      }
      usleep(50000);
    }
  }
}


void
on_global_iso_restart_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  on_global_iso_stop_clicked(GTK_BUTTON(lookup_widget(commander_window,"global_iso_stop")),NULL);
  on_global_iso_start_clicked(GTK_BUTTON(lookup_widget(commander_window,"global_iso_start")),NULL);
}


void
on_global_iso_start_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
  int i;
  for (i=0;i<camera_num;i++) {
    //fprintf(stderr,"Trying to start camera %d\n",i);
    if (misc_infos[i].is_iso_on!=DC1394_TRUE) {
      if (dc1394_start_iso_transmission(camera[i].handle,camera[i].id)!=DC1394_SUCCESS) {
	MainError("Could not start ISO transmission");
      }
      else {
	//fprintf(stderr," ISO started for camera %d\n",i);
	misc_infos[i].is_iso_on=DC1394_TRUE;
      }
      if (i==current_camera) {
	UpdateIsoFrame();
	UpdateTransferStatusFrame();
      }
      usleep(50000);
    }
  }
}
