/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
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
#include "build_windows.h"
#include "update_windows.h"
#include "definitions.h"
#include "capture.h"
#include "callback_proc.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

#define EEPROM_CNFG 0xF00U
#define TEST_CNFG 0xF04U
#define CCR_BASE  0xFFFFF0F00000ULL

extern GtkWidget *capture_window;
extern GtkWidget *format7_window;
extern GtkWidget *status_window;
extern GtkWidget *temperature_window;
extern GtkWidget *about_window;
extern GtkWidget *commander_window;
extern GtkWidget *aperture_window;
extern GtkWidget *color_window;
extern GtkWidget *porthole_window;
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
extern porthole_info pi;
extern capture_info ci;
extern isothread_info it;
extern guint gCaptureIdleID;

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
on_load_setup_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog = create_file_selector_window();
  gtk_window_set_title (GTK_WINDOW (dialog), "Select a setup file to load...");
  gtk_widget_show(dialog);

  // to be continued...
}


void
on_save_setup_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // TO BE FILLED
}


void
on_save_setup_as_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *dialog = create_file_selector_window();
  gtk_window_set_title (GTK_WINDOW (dialog), "Save setup as...");
  gtk_widget_show(dialog);

  // to be continued...
}


void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_exit(0);
}


void
on_porthole_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

  gtk_widget_show (porthole_window);
  pi.is_open=TRUE;
  if (uiinfo->overlay_power==1)
    PortholeStartThread();
}

gboolean
on_porthole_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  PortholeStopThread();
  pi.is_open = FALSE;
  gtk_widget_hide(porthole_window);
  return TRUE;
}

void
on_color_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (color_window);
}


void
on_aperture_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (aperture_window);
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


gboolean
on_file_selector_window_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  GtkWidget *dialog = gtk_widget_get_toplevel(widget);
  gtk_widget_destroy(dialog);
  return FALSE;
}

void
on_f0_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_160x120_YUV444,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f0_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_320x240_YUV422,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f0_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_640x480_YUV411,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f0_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_640x480_YUV422,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f0_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_640x480_RGB,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f0_mode5_ativate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_640x480_MONO,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f1_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_800x600_YUV422,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_800x600_RGB,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_800x600_MONO,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1024x768_YUV422,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1024x768_RGB,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1024x768_MONO,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f2_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1280x960_YUV422,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1280x960_RGB,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1280x960_MONO,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1600x1200_YUV422,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1600x1200_RGB,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1600x1200_MONO,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f6_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_EXIF,FORMAT_STILL_IMAGE);
}


void
on_f7_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_0,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_1,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_2,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_3,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_4,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_5,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_6,FORMAT_SCALABLE_IMAGE_SIZE);
}


void
on_f7_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_FORMAT7_7,FORMAT_SCALABLE_IMAGE_SIZE);
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
  int err;
  IsoFlowCheck();
  err=dc1394_set_video_framerate(camera->handle, camera->id, (int)user_data+FRAMERATE_MIN);
  if (!err) MainError("Could not set framerate");
  else
    misc_info->framerate=(int)user_data;
  IsoFlowResume();
}


void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  int err;
  err=dc1394_camera_on(camera->handle, camera->id);
  if (!err) MainError("Could not set camera 'on'");
}


void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  int err;
  err=dc1394_camera_off(camera->handle, camera->id);
  if (!err) MainError("Could not set camera 'off'");
}


void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  int err;
  err=dc1394_init_camera(camera->handle, camera->id);
  if (!err) MainError("Could not initilize camera");
}

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  int err;
  err=dc1394_set_trigger_polarity(camera->handle,camera->id,togglebutton->active);
  if (!err) MainError("Cannot set trigger polarity");
  else feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_polarity=(int)togglebutton->active;
}


void
on_pan_op_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button, FEATURE_PAN);
}


void
on_pan_power_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_PAN);
}


void
on_tilt_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_TILT);
}


void
on_tilt_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_TILT);
}


void
on_zoom_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_ZOOM);
}


void
on_zoom_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_ZOOM);
}


void
on_focus_op_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_FOCUS);
}


void
on_focus_power_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_FOCUS);
}


void
on_brightness_op_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_BRIGHTNESS);
}


void
on_brightness_power_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_BRIGHTNESS);
}


void
on_gamma_op_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_GAMMA);
}


void
on_gamma_power_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_GAMMA);
}


void
on_saturation_op_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_SATURATION);
}


void
on_saturation_power_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_SATURATION);
}


void
on_hue_op_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_HUE);
}


void
on_hue_power_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_HUE);
}


void
on_whitebal_power_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_WHITE_BALANCE);
}


void
on_whitebal_op_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_WHITE_BALANCE);
}


void
on_sharpness_op_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_SHARPNESS);
}


void
on_sharpness_power_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_SHARPNESS);
}


void
on_exposure_power_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_EXPOSURE);
}


void
on_exposure_op_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_EXPOSURE);
}


void
on_iris_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_IRIS);
}


void
on_iris_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_IRIS);
}


void
on_shutter_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_SHUTTER);
}


void
on_shutter_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_SHUTTER);
}


void
on_gain_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_GAIN);
}


void
on_gain_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_GAIN);
}


void
on_filter_power_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_OPTICAL_FILTER);
}


void
on_filter_op_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_OPTICAL_FILTER);
}


void
on_pan_man_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_PAN);
}


void
on_tilt_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_TILT);
}


void
on_zoom_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_ZOOM);
}


void
on_focus_man_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_FOCUS);
}


void
on_brightness_man_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_BRIGHTNESS);
}


void
on_gamma_man_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_GAMMA);
}


void
on_saturation_man_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_SATURATION);
}


void
on_hue_man_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_HUE);
}


void
on_whitebal_man_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_WHITE_BALANCE);
}


void
on_sharpness_man_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_SHARPNESS);
}


void
on_exposure_man_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_EXPOSURE);
}


void
on_iris_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_IRIS);
}


void
on_shutter_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_SHUTTER);
}


void
on_gain_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_GAIN);
}


void
on_filter_man_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_OPTICAL_FILTER);
}


void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  int err;
  err=dc1394_set_trigger_mode(camera->handle, camera->id, (int)user_data);
  if (!err) MainError("Could not set trigger mode");
  else feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode=(int)user_data;
  //UpdateTriggerFrame();
}

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  int err;
  err=dc1394_feature_on_off(camera->handle, camera->id, FEATURE_TRIGGER, togglebutton->active);
  if (!err) MainError("Could not set external trigger source");
  else feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on=togglebutton->active;
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
on_status_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (status_window);
}


void
on_capture_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (capture_window);
}


void
on_capqual_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_CAPTURE_QUALITY);
}


void
on_capqual_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_CAPTURE_QUALITY);
}


void
on_capqual_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_CAPTURE_QUALITY);
}


void
on_capsize_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_CAPTURE_SIZE);
}


void
on_capsize_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_CAPTURE_SIZE);
}


void
on_capsize_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_CAPTURE_SIZE);
}

void
on_capture_start_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = create_capture_multi_dialog();
    gtk_widget_show(dialog);
}


void
on_capture_stop_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_idle_remove(gCaptureIdleID);
  capture_multi_stop();
  gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(button), "capture_start")), TRUE);
  gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(button), "capture_stop")), FALSE);
  gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(button), "capture_single")), TRUE);
}


void
on_capture_single_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
  if (capture_single_frame()) {
    GtkWidget *dialog = create_save_single_dialog();
    gtk_widget_show(dialog);
  }
}

void
on_temp_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  PowerRangeProcedure(togglebutton,FEATURE_TEMPERATURE);
}


void
on_temp_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  ManRangeProcedure(togglebutton,FEATURE_TEMPERATURE);
}


void
on_temp_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  OpRangeProcedure(button,FEATURE_TEMPERATURE);
}


void
on_temp_window_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (temperature_window);
}


void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  int err;
  //int temp;
  err=dc1394_memory_load(camera->handle,camera->id, misc_info->load_channel);
  if (!err) MainError("Cannot load memory channel");
  //err=dc1394_get_memory_load_ch(camera->handle,camera->id, &temp);
  UpdateAllWindows();

}

void
on_save_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{ 
  int err, timeout;
  dc1394bool_t boolean;
  err=dc1394_set_memory_save_ch(camera->handle,camera->id, misc_info->save_channel);
  if (!err) MainError("Could not set memory save channel");
  else
    { 
      err=dc1394_memory_save(camera->handle,camera->id);
      if (!err) MainError("Could not save setup to memory channel");
      else
	{
	  err=dc1394_get_memory_save_ch(camera->handle,camera->id,&timeout);
	  if (!err) MainError("Could not query memory save channel");
	  else
	    {
	      boolean=TRUE;
	      timeout=LOOP_RETRIES;
	      while(boolean & (timeout>0))
		{ // wait for save to be completed:
		  usleep(LOOP_SLEEP);
		  err=dc1394_is_memory_save_in_operation(camera->handle,camera->id, &boolean);
		  if (!err) MainError("Could not query if memory save is in operation");
		  timeout--;
		}
	      if (timeout==0)
		MainError("Save operation function timed-out!\n");
	    }
	}
    }
}

void
on_iso_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  //int err;

  if (!IsoStartThread())
  //err=dc1394_start_iso_transmission(camera->handle,camera->id);
    //if (!err)
    MainError("Could not start ISO transmission");
  else
    {
      misc_info->is_iso_on=DC1394_TRUE;
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget(capture_window, "capture_start")), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget(capture_window, "capture_single")), TRUE);
      UpdateIsoFrame();
    }
  
  if ((uiinfo->overlay_power>0)&&pi.is_open)
    PortholeStartThread();

  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  int err;

  PortholeStopThread();
  IsoStopThread();

  err=dc1394_stop_iso_transmission(camera->handle,camera->id);
  if (!err)
    MainError("Could not stop ISO transmission");
  else
    {
      misc_info->is_iso_on=DC1394_FALSE;
      gtk_widget_set_sensitive( lookup_widget(capture_window, "capture_start"), FALSE); // added by DDouxchamps
      gtk_widget_set_sensitive( lookup_widget(capture_window, "capture_single"), FALSE);//
      UpdateIsoFrame();
    }
  //fprintf(stderr,"ISO stopped\n");
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
  // set current camera pointers: 
  SelectCamera((int)user_data);

  // redraw all:
  BuildAllWindows();
  UpdateAllWindows();
}

void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  //TODO
  //format7_info->edit_mode=...;
}

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  //TODO
}

void
on_f0_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_640x480_MONO16,FORMAT_VGA_NONCOMPRESSED);
}


void
on_f1_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_800x600_MONO16,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f1_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1024x768_MONO16,FORMAT_SVGA_NONCOMPRESSED_1);
}


void
on_f2_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1280x960_MONO16,FORMAT_SVGA_NONCOMPRESSED_2);
}


void
on_f2_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  ChangeModeAndFormat(MODE_1600x1200_MONO16,FORMAT_SVGA_NONCOMPRESSED_2);
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
  int err;

  switch((int)user_data)
    {
      case FEATURE_TEMPERATURE:
	err=dc1394_set_temperature(camera->handle,camera->id,adj->value);
	if (!err) MainError("Could not set temperature");
	else feature_set->feature[FEATURE_TEMPERATURE-FEATURE_MIN].target_value=adj->value;
	break;
      case FEATURE_WHITE_BALANCE+BU*4: // why oh why is there a *4?
	err=dc1394_set_white_balance(camera->handle,camera->id,adj->value, feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value);
	if (!err) MainError("Could not set B/U white balance");
	else feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value=adj->value;
	break;
      case FEATURE_WHITE_BALANCE+RV*4: // why oh why is there a *4?
	err=dc1394_set_white_balance(camera->handle,camera->id,feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].BU_value, adj->value);
	if (!err) MainError("Could not set R/V white balance");
	else feature_set->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN].RV_value=adj->value;
	break;
      default: // includes trigger_count
	err=dc1394_set_feature_value(camera->handle,camera->id,(int)user_data,adj->value);
	if (!err) MainError("Could not set feature");
	else feature_set->feature[(int)user_data-FEATURE_MIN].value=adj->value;
    }
}

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data)
{
  int err;
  GtkAdjustment* adj2;

  //printf("User data: %d\n",(int)(int*)(int)user_data);
  switch((int)user_data)
    {
      //printf("User data: %d\n",(int)user_data);
      case FORMAT7_SIZE_X:
	err=dc1394_set_format7_image_size(camera->handle,camera->id, format7_info->edit_mode,
					  adj->value, format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_y);
	if (!err) MainError("Could not set Format7 image size");
	else 
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_x=adj->value;

	    // adjust the pos_x adjustment so that (size_x+pos_x)<=max_size_x:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hposition_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_x-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	break;

      case FORMAT7_SIZE_Y:
	err=dc1394_set_format7_image_size(camera->handle,camera->id, format7_info->edit_mode,
					  format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_x, adj->value);
	if (!err) MainError("Could not set Format7 image size");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].size_y=adj->value;
	    // adjust the pos_y adjustment so that (size_y+pos_y)<=max_size_y:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vposition_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_y-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	break;

      case FORMAT7_POS_X:
	err=dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode,
					      adj->value, format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_y);
	if (!err) MainError("Could not set Format7 image position");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_x=adj->value;
	    // adjust the size_x adjustment so that (size_x+pos_x)<=max_size_x:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hsize_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_x-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	break;

      case FORMAT7_POS_Y:
	err=dc1394_set_format7_image_position(camera->handle,camera->id, format7_info->edit_mode,
					      format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_x, adj->value);
	if (!err) MainError("Cannot set Format7 image position");
	else
	  {
	    format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].pos_y=adj->value;
	    // adjust the size_y adjustment so that (size_y+pos_y)<=max_size_y:
	    adj2=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vsize_scale")));
	    adj2->upper=format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_size_y-adj->value;
	    gtk_signal_emit_by_name(GTK_OBJECT (adj2), "changed");
	  }
	break;

      default:
	MainError("Bad FORMAT7 scale ID passed to 'on_format7_value_changed'\n");
	break;
    }
}

void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // FUTURE FEATURE
}


void
on_format6_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // FUTURE FEATURE
}


gboolean
on_save_single_dialog_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(widget);
    gtk_widget_destroy(dialog);

    return FALSE;
}


void
on_save_single_dialog_ok_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *window = lookup_widget(capture_window, "capture_window");
    gchar *filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog));
    gtk_widget_hide(dialog);
    save_single_frame(filename);
    
    if (ci.ftp_enable) 
    {
      ci.ftp_address = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_address")));
      ci.ftp_path = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_path")));
      ci.ftp_user = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_user")));
      ci.ftp_passwd = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_passwd")));
      ftp_single_frame(filename, ci.ftp_address, ci.ftp_path, filename, ci.ftp_user, ci.ftp_passwd);
    }
    gtk_widget_destroy(dialog);
}


void
on_save_single_dialog_cancel_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(dialog);
}


gboolean
on_capture_multi_dialog_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(widget);
    gtk_widget_destroy(dialog);

     return FALSE;
}


void
on_capture_multi_dialog_ok_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
    gchar *filename;
    GtkWidget *window = lookup_widget(capture_window, "capture_window");
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog));
    gtk_widget_hide(dialog);
    if (capture_multi_start(filename)) {
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(window), "capture_start")), FALSE);
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(window), "capture_stop")), TRUE);
      gtk_widget_set_sensitive( GTK_WIDGET(lookup_widget( GTK_WIDGET(window), "capture_single")), FALSE);

      if (ci.ftp_enable) 
      {
        ci.ftp_address = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_address")));
        ci.ftp_path = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_path")));
        ci.ftp_user = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_user")));
        ci.ftp_passwd = gtk_entry_get_text( GTK_ENTRY(lookup_widget(window, "entry_capture_ftp_passwd")));
        ftp_single_frame(filename, ci.ftp_address, ci.ftp_path, filename, ci.ftp_user, ci.ftp_passwd);
      }

      if (ci.frequency == CAPTURE_FREQ_IMMEDIATE)
        gCaptureIdleID = gtk_idle_add( capture_idler, NULL);
      else
        {
          ci.period = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(lookup_widget( GTK_WIDGET(window), "spinbutton_capture_freq_periodic")));
          gCaptureIdleID = gtk_timeout_add( ci.period * 1000, capture_idler, NULL);
        }
    }
    gtk_widget_destroy(dialog);
}


void
on_capture_multi_dialog_cancel_clicked (GtkButton       *button,
                                        gpointer         user_data)
{
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(dialog);
}


void
on_overlay_button_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

  uiinfo->overlay_power=togglebutton->active;
  if (uiinfo->overlay_power>0)
    {
      IsoStartThread();
      PortholeStartThread();
      UpdateIsoFrame();
    }
  else
    PortholeStopThread();
}


void
on_test_pattern_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

    int err;
    quadlet_t value;
    IsoFlowCheck();
    
    value= htonl(0x12345678ULL);
    err= raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + EEPROM_CNFG, 4, &value);
    if (!err) MainError("Could not set test pattern registers");
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
    err= raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + TEST_CNFG, 4, &value);
    if (!err) MainError("Could not set test pattern registers");
    value= htonl(0x00000000ULL);
    err= raw1394_write(camera->handle, 0xffc0 | camera->id, CCR_BASE + EEPROM_CNFG, 4, &value);
    if (!err) MainError("Could not set test pattern registers");

    BuildFpsMenu();
    UpdateTriggerFrame();
    IsoFlowResume();  
}


void
on_radiobutton_capture_freq_immediate_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
  ci.frequency = CAPTURE_FREQ_IMMEDIATE;
}


void
on_radiobutton_capture_freq_periodioc_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
  ci.frequency = CAPTURE_FREQ_PERIODIC;
}


void
on_radiobutton_capture_mode_seq_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
  ci.mode = CAPTURE_MODE_SEQUENTIAL;
}


void
on_radiobutton_capture_mode_over_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
  ci.mode = CAPTURE_MODE_OVERWRITE;
}


void
on_checkbutton_capture_ftp_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

#ifdef HAVE_FTPLIB
  UpdateFTPFrame();
#else
  if (gtk_toggle_button_get_active(togglebutton)) MessageBox("You must have FtpLib to use FTP!");
  gtk_toggle_button_set_active( togglebutton, FALSE);
#endif
}

