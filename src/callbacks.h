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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

gboolean
on_commander_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_file_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_load_setup_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_setup_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_setup_as_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_porthole_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_color_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_aperture_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format_7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_porthole_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_color_window_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_aperture_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_file_selector_window_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_configure_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_about_window_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_commander_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_f0_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f0_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f0_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f0_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f0_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f0_mode5_ativate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f6_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode0_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode2_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode3_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode4_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode5_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f7_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_fps_menu_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_fps_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_window_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_capture_start_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_capture_stop_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_capture_single_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_pan_op_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_pan_power_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tilt_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tilt_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_zoom_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_zoom_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_focus_op_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_focus_power_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_brightness_op_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_brightness_power_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_gamma_op_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_gamma_power_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_saturation_op_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_saturation_power_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_hue_op_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_hue_power_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_whitebal_power_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_whitebal_op_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_sharpness_op_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_sharpness_power_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_exposure_power_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_exposure_op_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_iris_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_iris_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_shutter_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_shutter_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_gain_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_gain_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_filter_power_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_filter_op_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_pan_man_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_pan_auto_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tilt_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_zoom_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_focus_man_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_brightness_man_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_gamma_man_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_saturation_man_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_hue_man_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_whitebal_man_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_sharpness_man_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_exposure_man_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_iris_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_shutter_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_gain_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_filter_man_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_memory_channel_activate              (GtkMenuItem     *menuitem,
				      gpointer         user_data);

void
on_status_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_capture_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_capqual_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_capqual_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_capqual_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_capsize_man_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_capsize_op_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_capsize_power_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_temp_power_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_temp_man_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_temp_op_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_temp_window_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_set_all_auto_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_set_all_man_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_lock_setup_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_iso_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_iso_restart_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_camera_select_activate              (GtkMenuItem     *menuitem,
					gpointer         user_data);

gboolean
on_porthole_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_porthole_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_porthole_window_delete_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_porthole_window_size_request        (GtkWidget       *widget,
                                        GtkRequisition  *requisition,
                                        gpointer         user_data);

void
on_porthole_window_size_allocate       (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data);

void
on_f0_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f1_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode6_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_f2_mode7_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format7_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_scale_value_changed             ( GtkAdjustment    *adj,
				     gpointer         user_data);

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data);

void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data);

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					    gpointer         user_data);

void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_format6_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_save_single_dialog_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_save_single_dialog_ok_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_single_dialog_cancel_clicked   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_capture_multi_dialog_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_capture_multi_dialog_ok_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_capture_multi_dialog_cancel_clicked (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_save_single_dialog_delete_event     (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_capture_multi_dialog_delete_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_overlay_button_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_test_pattern_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

#endif
