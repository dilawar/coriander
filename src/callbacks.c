/*
 * Copyright (C) 2000-2002 Damien Douxchamps  <douxchamps@ieee.org>
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

#include <gnome.h>
#include "callbacks.h"
#include <netinet/in.h>
#include "interface.h"
#include "support.h"
#include "build_menus.h"
#include "update_frames.h"
#include "update_ranges.h"
#include "build_windows.h"
#include "update_windows.h"
#include "definitions.h"
#include "preferences.h" 
#include "tools.h"
#include "thread_iso.h"
#include "thread_display.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "thread_base.h"
#include <libdc1394/dc1394_control.h>

#define EEPROM_CNFG       0xF00U
#define TEST_CNFG         0xF04U
#define CCR_BASE          0xFFFFF0F00000ULL

extern GtkWidget *format7_window;
extern GtkWidget *about_window;
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
extern UIInfo *uiinfo;
extern UIInfo *uiinfos;
extern int current_camera;
extern PrefsInfo preferences; 
extern int silent_ui_update;
extern char* feature_scale_list[NUM_FEATURES];

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
on_format_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_format_0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}



void
on_format_1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}

void
on_format_2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_format_6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_format_7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  about_window = create_about_window ();
  gtk_widget_show (about_window);
}


void
on_fps_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_fps_activate                    (GtkMenuItem     *menuitem,
				    gpointer         user_data)
{
  int state[5];
  
  IsoFlowCheck(state);
    
  if(!dc1394_set_video_framerate(camera->handle, camera->id, (int)user_data))
    MainError("Could not set framerate");
  else
    misc_info->framerate=(int)user_data;

  IsoFlowResume(state);
}


void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if(!dc1394_camera_on(camera->handle, camera->id))
    MainError("Could not set camera 'on'");
}


void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  if(!dc1394_camera_off(camera->handle, camera->id))
    MainError("Could not set camera 'off'");
}


void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if (!dc1394_init_camera(camera->handle, camera->id))
    MainError("Could not initilize camera");
}

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!dc1394_set_trigger_polarity(camera->handle,camera->id,togglebutton->active))
    MainError("Cannot set trigger polarity");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_polarity=(int)togglebutton->active;
}


void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  if (!dc1394_set_trigger_mode(camera->handle, camera->id, (int)user_data))
    MainError("Could not set trigger mode");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode=(int)user_data;
  UpdateTriggerFrame();
}

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!dc1394_feature_on_off(camera->handle, camera->id, FEATURE_TRIGGER, togglebutton->active))
    MainError("Could not set external trigger source");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on=togglebutton->active;
  UpdateTriggerFrame();
}

void
on_trigger_value_changed               (GtkAdjustment    *adj,
                                        gpointer         user_data)
{
  if (!dc1394_set_feature_value(camera->handle, camera->id, FEATURE_TRIGGER, adj->value))
    MainError("Could not set external trigger count");
  else
    feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].value=adj->value;
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
  if (!dc1394_memory_load(camera->handle,camera->id, misc_info->load_channel))
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

  if (!dc1394_set_memory_save_ch(camera->handle,camera->id, misc_info->save_channel))
    MainError("Could not set memory save channel");
  else
    { 
      if (!dc1394_memory_save(camera->handle,camera->id))
	MainError("Could not save setup to memory channel");
      else
	{
	  while ((value==DC1394_TRUE)
		 &&(timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) )
	    {
	      usleep(step);
	      if (!dc1394_is_memory_save_in_operation(camera->handle,camera->id, &value))
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
  if (!dc1394_start_iso_transmission(camera->handle,camera->id))
    MainError("Could not start ISO transmission");
  else
    {
      misc_info->is_iso_on=DC1394_TRUE;
      UpdateIsoFrame();
    }
  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (!dc1394_stop_iso_transmission(camera->handle,camera->id))
    MainError("Could not stop ISO transmission");
  else
    {
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
  // set current camera pointers:
  SelectCamera((int)user_data);

  if (uiinfo->want_to_display>0)
    DisplayStartThread();

  // redraw all:
  BuildAllWindows();
  UpdateAllWindows();
}

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data)
{
  int bpp;
  int state[5];

  IsoFlowCheck(state);
  if (dc1394_set_format7_byte_per_packet(camera->handle, camera->id, 
					 format7_info->edit_mode, (int)adj->value)!=DC1394_SUCCESS)
    MainError("Could not change Format7 bytes per packet");
  else
    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].bpp=adj->value;

  dc1394_query_format7_byte_per_packet(camera->handle,camera->id,format7_info->edit_mode,&bpp);

  IsoFlowResume(state);
  //fprintf(stderr,"bpp: %d (should set to %d)\n",bpp, (int)adj->value);
}


void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data)
{
  format7_info->edit_mode=(int)user_data;
  BuildFormat7Window();
}

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					    gpointer         user_data)
{
  if (dc1394_set_format7_color_coding_id(camera->handle, camera->id, format7_info->edit_mode, (int)user_data)!=DC1394_SUCCESS)
    MainError("Could not change Format7 color coding");
  else
    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].color_coding_id=(int)user_data;

  // update bpp range here.

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

  switch((int)user_data)
    {
      case FEATURE_TEMPERATURE:
	if (!dc1394_set_temperature(camera->handle,camera->id,adj->value))
	  MainError("Could not set temperature");
	else
	  feature_set->feature[FEATURE_TEMPERATURE-FEATURE_MIN].target_value=adj->value;
	break;
      case FEATURE_WHITE_BALANCE+BU*4: // why oh why is there a *4?
	if (!dc1394_set_white_balance(camera->handle,camera->id,adj->value, feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value))
	  MainError("Could not set B/U white balance");
	else
	  feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value=adj->value;
	break;
      case FEATURE_WHITE_BALANCE+RV*4: // why oh why is there a *4?
	if (!dc1394_set_white_balance(camera->handle,camera->id,feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value, adj->value))
	  MainError("Could not set R/V white balance");
	else
	  feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value=adj->value;
	break;
      default: // includes trigger_count
	if (!dc1394_set_feature_value(camera->handle,camera->id,(int)user_data,adj->value))
	  MainError("Could not set feature");
	else
	  feature_set->feature[(int)user_data-FEATURE_MIN].value=adj->value;
    }
}

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data)
{
  GtkAdjustment* adj2;
  int state[5];
  int step;

  switch((int)user_data)
    {
      case FORMAT7_SIZE_X:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(state);
	step=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].step_x;
	adj->value=((int)adj->value/step)*step;
	if (!dc1394_set_format7_image_size(camera->handle,camera->id, format7_info->edit_mode,
					   adj->value, format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_y))
	  MainError("Could not set Format7 image size");
	else 
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_x=adj->value;
	    // adjust the pos_x adjustment so that (size_x+pos_x)<=max_size_x:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hposition_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_x-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(state);
	break;

      case FORMAT7_SIZE_Y:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(state);
	step=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].step_y;
	adj->value=((int)adj->value/step)*step;
	if (!dc1394_set_format7_image_size(camera->handle,camera->id, format7_info->edit_mode,
					   format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_x, adj->value))
	  MainError("Could not set Format7 image size");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_y=adj->value;
	    // adjust the pos_y adjustment so that (size_y+pos_y)<=max_size_y:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vposition_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_y-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(state);
	break;

      case FORMAT7_POS_X:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(state);
	step=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].step_x;
	adj->value=((int)adj->value/step)*step;
	if (!dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode,
					      adj->value, format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_y))
	  MainError("Could not set Format7 image position");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_x=adj->value;
	    // adjust the size_x adjustment so that (size_x+pos_x)<=max_size_x:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hsize_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_x-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(state);
	break;

      case FORMAT7_POS_Y:
	if (format7_info->edit_mode==misc_info->mode) IsoFlowCheck(state);
	step=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].step_y;
	adj->value=((int)adj->value/step)*step;
	if (!dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode,
					      format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_x, adj->value))
	  MainError("Cannot set Format7 image position");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_y=adj->value;
	    // adjust the size_y adjustment so that (size_y+pos_y)<=max_size_y:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vsize_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_y-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	if (format7_info->edit_mode==misc_info->mode) IsoFlowResume(state);
	break;

      default:
	MainError("Bad FORMAT7 scale ID passed to 'on_format7_value_changed'\n");
	break;
    }

  // update bpp range here.
  UpdateFormat7BppRange();
}


void
on_format6_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // FUTURE FEATURE
}


void
on_test_pattern_toggled               (GtkToggleButton *togglebutton,
                                        gpointer        user_data)
{
    quadlet_t value;
    int state[5];

    IsoFlowCheck(state);
    
    value= htonl(0x12345678ULL);
    if (!raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + EEPROM_CNFG, 4, &value))
      MainError("Could not set test pattern registers");
    if (uiinfo->test_pattern==0)
      {
	value= htonl(0x80000000ULL);
	uiinfo->test_pattern=1;
      }
    else
      {
	value= htonl(0x00000000ULL);
	uiinfo->test_pattern=0;
      }
    if (!raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + TEST_CNFG, 4, &value))
    MainError("Could not set test pattern registers");
    value= htonl(0x00000000ULL);
    if (!raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + EEPROM_CNFG, 4, &value))
    MainError("Could not set test pattern registers");

    BuildFpsMenu();
    UpdateTriggerFrame();
    IsoFlowResume(state);  
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
  if (!silent_ui_update)
    {
      if (togglebutton->active)
	IsoStartThread();
      else
	CleanThreads(CLEAN_MODE_UI_UPDATE_NOT_ISO);
    }

}


void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update)
    {
      if (togglebutton->active)
	{
	  if (GetService(SERVICE_ISO,current_camera)==NULL)
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
	  uiinfo->want_to_display=1;
	  DisplayStartThread();
	} 
      else
	{
	  DisplayStopThread(current_camera);
	  uiinfo->want_to_display=0;
	} 
    }
}


void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update)
    {
      if (togglebutton->active)
	{
	  if (GetService(SERVICE_ISO,current_camera)==NULL)
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
	  SaveStartThread();
	}
      else
	SaveStopThread();
    }
}


void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update)
    {
      if (togglebutton->active)
	{
	  if (GetService(SERVICE_ISO,current_camera)==NULL)
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
	  FtpStartThread();
	}
      else
	FtpStopThread();
    }
}


void
on_service_real_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update)
    {
      if (togglebutton->active)
	{
	  if (GetService(SERVICE_ISO,current_camera)==NULL)
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"service_iso")), TRUE);
	  RealStartThread();
	}
      else
	RealStopThread();
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

  switch (action)
    {
    case RANGE_MENU_OFF : // ============================== OFF ==============================
      if (!dc1394_feature_on_off(camera->handle, camera->id, feature, FALSE))
	MainError("Could not set feature on/off");
      else
	{
	  feature_set->feature[feature-FEATURE_MIN].is_on=FALSE;
	  UpdateRange(commander_window,feature);
	}
      break;
    case RANGE_MENU_MAN : // ============================== MAN ==============================
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable)
	{
	  if (!dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE))
	    {
	      MainError("Could not set feature on");
	      break;
	    }
	  else
	    feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
	}
      if (!dc1394_auto_on_off(camera->handle, camera->id, feature, FALSE))
	MainError("Could not set manual mode");
      else
	{
	  feature_set->feature[feature-FEATURE_MIN].auto_active=FALSE;
	  UpdateRange(commander_window,feature);
	}
      break;
    case RANGE_MENU_AUTO : // ============================== AUTO ==============================
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable)
	{
	  if (!dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE))
	    {
	      MainError("Could not set feature on");
	      break;
	    }
	  else
	    feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
	}
      if (!dc1394_auto_on_off(camera->handle, camera->id, feature, TRUE))
	MainError("Could not set auto mode");
      else
	{
	  feature_set->feature[feature-FEATURE_MIN].auto_active=TRUE;
	  UpdateRange(commander_window,feature);
	}
      break;
    case RANGE_MENU_SINGLE : // ============================== SINGLE ==============================
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable)
	{
	  if (!dc1394_feature_on_off(camera->handle, camera->id, feature, TRUE))
	    {
	      MainError("Could not set feature on");
	      break;
	    }
	  else
	    feature_set->feature[feature-FEATURE_MIN].is_on=TRUE;
	}
      step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);
      if (!dc1394_start_one_push_operation(camera->handle, camera->id, feature))
	MainError("Could not start one-push operation");
      else
	{
	  SetScaleSensitivity(GTK_WIDGET(menuitem),feature,FALSE);
	  while ((value==DC1394_TRUE) 
		 && (timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) )
	    {
	      usleep(step);
	      if (!dc1394_is_one_push_in_operation(camera->handle, camera->id, feature, &value))
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
  preferences.display_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_display_period")));
  gnome_config_set_int("coriander/display/period",preferences.display_period);
  gnome_config_sync();
}

void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.save_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_save_period")));
  gnome_config_set_int("coriander/save/period",preferences.save_period);
  gnome_config_sync();
}


void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_ftp_period")));
  gnome_config_set_int("coriander/ftp/period",preferences.ftp_period);
  gnome_config_sync();
}


void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_address=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_ftp_address")));
  gnome_config_set_string("coriander/ftp/address",preferences.ftp_address);
  gnome_config_sync();
}


void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_path=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_ftp_path")));
  gnome_config_set_string("coriander/ftp/path",preferences.ftp_path);
  gnome_config_sync();

}


void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_user=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_ftp_user")));
  gnome_config_set_string("coriander/ftp/user",preferences.ftp_user);
  gnome_config_sync();

}


void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_password=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_ftp_password")));
  gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  gnome_config_sync();
}


void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.ftp_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_ftp_filename")));
  gnome_config_set_string("coriander/ftp/filename",preferences.ftp_filename);
  gnome_config_sync();
}


void
on_prefs_real_address_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_address=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_address")));
  gnome_config_set_string("coriander/real/address",preferences.real_address);
  gnome_config_sync();
}

void
on_prefs_save_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.save_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_save_filename")));
  gnome_config_set_string("coriander/save/filename",preferences.save_filename);
  gnome_config_sync();
}



void
on_prefs_real_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_filename=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_filename")));
  gnome_config_set_string("coriander/real/filename",preferences.real_filename);
  gnome_config_sync();
}


void
on_prefs_real_user_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_user=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_user")));
  gnome_config_set_string("coriander/real/user",preferences.real_user);
  gnome_config_sync();
}


void
on_prefs_real_password_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_password=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_password")));
  gnome_config_set_string("coriander/real/password",preferences.real_password);
  gnome_config_sync();
}


void
on_prefs_real_port_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_port=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_real_port")));
  gnome_config_set_int("coriander/real/port",preferences.real_port);
  gnome_config_sync();

}


void
on_prefs_real_title_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_title=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_title")));
  gnome_config_set_string("coriander/real/title",preferences.real_title);
  gnome_config_sync();

}


void
on_prefs_real_author_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_author=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_author")));
  gnome_config_set_string("coriander/real/author",preferences.real_author);
  gnome_config_sync();

}


void
on_prefs_real_copyright_changed        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.real_copyright=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_real_copyright")));
  gnome_config_set_string("coriander/real/copyright",preferences.real_copyright);
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
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_save_filename")),filename);
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
on_prefs_real_quality_activate         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.real_quality=(int)user_data;
  gnome_config_set_int("coriander/real/quality",preferences.real_quality);
  gnome_config_sync();
}

void
on_prefs_real_compatibility_activate   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.real_compatibility=(int)user_data;
  gnome_config_set_int("coriander/real/compatibility",preferences.real_compatibility);
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
on_prefs_real_recordable_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_recordable=1;
  else
    preferences.real_recordable=0;
  gnome_config_set_int("coriander/real/recordable",preferences.real_recordable);
  gnome_config_sync();
}


void
on_prefs_real_audience_28k_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_28_MODEM);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_28_MODEM));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_56k_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_56_MODEM);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_56_MODEM));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_sisdn_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_SINGLE_ISDN);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_SINGLE_ISDN));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_disdn_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_DUAL_ISDN);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_DUAL_ISDN));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_lan_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_LAN_HIGH);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_LAN_HIGH));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_dsl256_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_256_DSL_CABLE);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_256_DSL_CABLE));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_dsl384_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_384_DSL_CABLE);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_384_DSL_CABLE));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}


void
on_prefs_real_audience_dsl512_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.real_audience=(preferences.real_audience | REAL_AUDIENCE_512_DSL_CABLE);
  else
    preferences.real_audience=(preferences.real_audience & (~REAL_AUDIENCE_512_DSL_CABLE));
  gnome_config_set_int("coriander/real/audience",preferences.real_audience);
  gnome_config_sync();
  //fprintf(stderr,"audience flags: 0x%x\n",preferences.real_audience);
}

void
on_prefs_video1394_device_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.video1394_device=gtk_entry_get_text(GTK_ENTRY(lookup_widget(preferences_window,"prefs_video1394_device")));
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

