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
on_exit_activate                       (GtkMenuItem     *menuitem, 
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
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
on_fps_activate                        (GtkMenuItem     *menuitem,
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
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_trigger_mode_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_trigger_value_changed               (GtkAdjustment    *adj,
                                        gpointer         user_data);

void
on_memory_channel_activate             (GtkMenuItem     *menuitem,
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

void
on_format7_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_scale_value_changed                 (GtkAdjustment    *adj,
				        gpointer         user_data);

void
on_format7_value_changed               (GtkAdjustment    *adj,
				        gpointer         user_data);

void
on_edit_format7_mode_activate          (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_edit_format7_color_activate         (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_test_pattern_toggled               (GtkToggleButton *togglebutton,
                                        gpointer        user_data);

void
on_prefs_update_power_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_preferences_window_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_prefs_save_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_close_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_update_value_changed          (GtkAdjustment    *adj,
					gpointer         user_data);

void
on_prefs_timeout_value_changed         (GtkAdjustment    *adj,
					gpointer         user_data);

void
on_service_iso_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_seq_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_scratch_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_choose_clicked           (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_get_filename_dialog_delete_event    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_get_filename_dialog_destroy_event   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_get_filename_dialog_destroy         (GtkObject       *object,
                                        gpointer         user_data);

void
on_get_filename_dialog_ok_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_get_filename_dialog_cancel_clicked  (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_ftp_seq_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_ftp_scratch_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_real_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_range_menu_activate                 (GtkMenuItem     *menuitem,
				        gpointer         user_data);

void
on_prefs_real_quality_activate         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_compatibility_activate   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_receive_method_activate       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_display_keep_ratio_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_recordable_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_28k_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_56k_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_sisdn_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_disdn_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_lan_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_dsl256_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_dsl384_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_real_audience_dsl512_toggled  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data);
#endif

void
on_key_bindings_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_prefs_save_video_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_convert_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_noconvert_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_name_camera_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_camera_name_text_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_video1394_device_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_display_period_changed        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_save_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_address_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_filename_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_user_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_password_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_port_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_title_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_author_changed           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_real_copyright_changed        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_receive_drop_frames_toggled   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
