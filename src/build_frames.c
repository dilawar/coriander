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


#include "build_frames.h"

extern GtkWidget *commander_window;
extern GtkWidget *preferences_window;
extern GtkWidget *absolute_settings_window;
extern PrefsInfo preferences;
extern camera_t* camera;

void
BuildCameraFrame(void)
{
  BuildCameraMenu();
}

void
BuildServiceFrame(void)
{
#ifdef HAVE_FTPLIB
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_ftp"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_ftp"),FALSE);
#endif
#ifdef HAVE_SDLLIB
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_display"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_display"),FALSE);
#endif
}

void
BuildTriggerFrame(void)
{

  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_frame"),TRUE);

  BuildTriggerModeMenu();
  BuildFpsMenu();
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"trigger_external")),
			       camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on);
}


void
BuildPowerFrame(void)
{
  quadlet_t basic_funcs;
  // these two functions are always present:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_reset"),TRUE);

  // activate if camera capable of power on/off:
  if (dc1394_query_basic_functionality(camera->camera_info.handle,camera->camera_info.id,&basic_funcs)!=DC1394_SUCCESS)
    MainError("Could not query basic functionalities");

  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_on"),(basic_funcs & 0x1<<16));
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_off"),(basic_funcs & 0x1<<16));

}


void
BuildMemoryFrame(void)
{
  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"memory_frame"),TRUE);

  // activate the mem channel menu:
  BuildMemoryChannelMenu();

}

void
BuildIsoFrame(void)
{
  // TODO: only if ISO capable
  if (dc1394_get_iso_status(camera->camera_info.handle,camera->camera_info.id,&camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
    MainError("Can't get ISO status");
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_start"),!camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_restart"),camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_stop"),camera->misc_info.is_iso_on);

}

void
BuildGlobalIsoFrame(void)
{
  // TODO: only if ISO capable
  gtk_widget_set_sensitive(lookup_widget(commander_window,"global_iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"global_iso_start"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"global_iso_restart"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"global_iso_stop"),TRUE);

}

void
BuildFormat7ModeFrame(void)
{
  BuildFormat7ModeMenu();
  BuildFormat7ColorMenu();
}

void
BuildCameraStatusFrame(void)
{ 
}

void
BuildTransferStatusFrame(void)
{
}

void
BuildPrefsSaveFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(commander_window,
							  "prefs_save_period"), preferences.save_period);
  // scratch
  switch(preferences.save_scratch) {
  case SAVE_SCRATCH_OVERWRITE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_save_scratch"),TRUE);
    break;
  case SAVE_SCRATCH_SEQUENTIAL:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_save_seq"),TRUE);
  case SAVE_SCRATCH_SEQUENCE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_save_video"),TRUE);
    break;
  }
  // scratch
  if (preferences.save_convert == SAVE_CONVERT_ON)
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_save_convert"),TRUE);
  else
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_save_noconvert"),TRUE);
  
  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_save_filename")), preferences.save_filename);
}

void
BuildPrefsGeneralFrame(void)
{
  
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_op_timeout_scale"), preferences.op_timeout);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_update_scale"), preferences.auto_update_frequency);
}

void
BuildPrefsFtpFrame(void)
{
#ifdef HAVE_FTPLIB
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(commander_window, "prefs_ftp_period"), preferences.ftp_period);
  // scratch
  switch(preferences.ftp_scratch) {
  case FTP_SCRATCH_OVERWRITE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_ftp_scratch"),TRUE);
    break;
  case FTP_SCRATCH_SEQUENTIAL:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(commander_window, "prefs_ftp_seq"),TRUE);
    break;
  }
  // file,... names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_ftp_filename")), preferences.ftp_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_ftp_address")), preferences.ftp_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_ftp_password")),preferences.ftp_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_ftp_path")), preferences.ftp_path);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_ftp_user")), preferences.ftp_user);

#else

  gtk_widget_set_sensitive(lookup_widget(commander_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"prefs_ftp_scratch_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"prefs_ftp_server_frame"),FALSE);

#endif

}

void
BuildPrefsDisplayFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(commander_window,"prefs_display_period"), preferences.display_period);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window,"prefs_display_keep_ratio")), preferences.display_keep_ratio);
}

void
BuildPrefsReceiveFrame(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  int video_ok=0;
  int k=0;
  struct stat statstruct;

  if (stat("/dev/video1394",&statstruct)==0) {
    // the device is there, check RW permissions
    if ((statstruct.st_mode&&S_IRUSR)&&(statstruct.st_mode&&S_IWUSR))
      video_ok=1;
  }
  // BUILD A NEW  OPTION_MENU:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(commander_window,"prefs_receive_method_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "prefs_receive_method_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,"table45")),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // always add a raw1394 item
  glade_menuitem = gtk_menu_item_new_with_label (_("RAW1394"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_prefs_receive_method_activate),
		      (int*)RECEIVE_METHOD_RAW1394); 
  preferences.receive_method2index[RECEIVE_METHOD_RAW1394]=k;
  k++;

  if (video_ok==1) {
    // 'video1394' menuitem optional addition:
    glade_menuitem = gtk_menu_item_new_with_label (_("VIDEO1394"));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			GTK_SIGNAL_FUNC (on_prefs_receive_method_activate),
			(int*)RECEIVE_METHOD_VIDEO1394); 
    preferences.receive_method2index[RECEIVE_METHOD_VIDEO1394]=k;
    k++;
  }
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(commander_window, "prefs_receive_method_menu")),
			      preferences.receive_method2index[preferences.receive_method]);

  gtk_entry_set_text(GTK_ENTRY(lookup_widget(commander_window, "prefs_video1394_device")), preferences.video1394_device);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(commander_window, "prefs_receive_dropframes")), preferences.video1394_dropframes);
}

void
BuildOptionFrame(void)
{
  pthread_mutex_lock(&camera->uimutex);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(commander_window, "mono16_bpp"),camera->bpp);
  pthread_mutex_unlock(&camera->uimutex);
  BuildBayerMenu();
  BuildBayerPatternMenu();
  BuildStereoMenu();
}


int
BuildAbsApertureFrame(void)
{
  int present=0;
  present+=BuildAbsControl(FEATURE_EXPOSURE);
  present+=BuildAbsControl(FEATURE_IRIS);
  present+=BuildAbsControl(FEATURE_SHUTTER);
  present+=BuildAbsControl(FEATURE_GAIN);
  return present;
}


int
BuildAbsColorFrame(void)
{
  int present=0;
  present+=BuildAbsControl(FEATURE_HUE);
  present+=BuildAbsControl(FEATURE_SATURATION);
  present+=BuildAbsControl(FEATURE_WHITE_BALANCE);
  return present;
}


int
BuildAbsLuminanceFrame(void)
{  
  int present=0;
  present+=BuildAbsControl(FEATURE_BRIGHTNESS);
  return present;
}


int
BuildAbsPositioningFrame(void)
{
  int present=0;
  present+=BuildAbsControl(FEATURE_ZOOM);
  present+=BuildAbsControl(FEATURE_FOCUS);
  present+=BuildAbsControl(FEATURE_TILT);
  present+=BuildAbsControl(FEATURE_PAN);
  return present;
}
