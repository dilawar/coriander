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
#include "support.h"
#include <sys/stat.h>
#include "definitions.h"
#include "build_menus.h"
#include "build_ranges.h"
#include "build_frames.h"
#include "update_frames.h"
#include "thread_ftp.h"
#include "thread_save.h"
#include "thread_display.h"
#include "thread_iso.h"
#include "preferences.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *commander_window;
extern GtkWidget *preferences_window;
extern dc1394_feature_set *feature_set;
extern dc1394_camerainfo *camera;
extern dc1394_miscinfo *misc_info;
extern PrefsInfo preferences;
extern int camera_num;
extern UIInfo* uiinfo;

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
#ifdef HAVE_REALLIB
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_real"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(commander_window,"service_real"),FALSE);
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
			       feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].is_on);
}


void
BuildPowerFrame(void)
{
  quadlet_t basic_funcs;
  int err;
  // these two functions are always present:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"power_reset"),TRUE);

  // activate if camera capable of power on/off:
  err=dc1394_query_basic_functionality(camera->handle,camera->id,&basic_funcs);
  if (err<0) MainError("Could not query basic functionalities");

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
  int err;
  // TODO: only if ISO capable
  err=dc1394_get_iso_status(camera->handle,camera->id,&misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_start"),!misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_restart"),misc_info->is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(commander_window,"iso_stop"),misc_info->is_iso_on);

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
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_save_period"), preferences.save_period);
  // scratch
  switch(preferences.save_scratch)
    {
    case SAVE_SCRATCH_OVERWRITE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_scratch"),TRUE);
      break;
    case SAVE_SCRATCH_SEQUENTIAL:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_save_seq"),TRUE);
      break;
    }

  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_save_filename")),
		     preferences.save_filename);
}

void
BuildPrefsGeneralFrame(void)
{
  
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_op_timeout_scale"),
			    preferences.op_timeout);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_update_scale"),
			    preferences.auto_update_frequency);
}

void
BuildPrefsFtpFrame(void)
{
#ifdef HAVE_FTPLIB
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,
							  "prefs_ftp_period"), preferences.ftp_period);
  // scratch
  switch(preferences.ftp_scratch)
    {
    case FTP_SCRATCH_OVERWRITE:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_scratch"),TRUE);
      break;
    case FTP_SCRATCH_SEQUENTIAL:
      gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window,
								   "prefs_ftp_seq"),TRUE);
      break;
    }
  // file,... names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_filename")),
		     preferences.ftp_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_address")),
		     preferences.ftp_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_password")),
		     preferences.ftp_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_path")),
		     preferences.ftp_path);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_ftp_user")),
		     preferences.ftp_user);

#else

  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_scratch_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_ftp_server_frame"),FALSE);

#endif

}

void
BuildPrefsRealFrame(void)
{
  GtkMenuItem* menuitem;
  GtkMenu* menu;
  GtkOptionMenu* option_menu;
  int i;

  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_quality");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<4;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_quality_activate),
			  (int*)i); 
    }

  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_compatibility");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<2;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_compatibility_activate),
			  (int*)i); 
    }

#ifdef HAVE_REALLIB
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_server_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_infos_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_stream_frame"),TRUE);

  // names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_filename")),
		     preferences.real_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_address")),
		     preferences.real_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_password")),
		     preferences.real_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_user")),
		     preferences.real_user);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_title")),
		     preferences.real_title);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_author")),
		     preferences.real_author);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_real_copyright")),
		     preferences.real_copyright);

  // port
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window, "prefs_real_port"),
			    preferences.real_port);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_real_quality")),
			      preferences.real_quality);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_real_compatibility")),
			      preferences.real_compatibility);

  // recordable?
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_recordable")),
			       preferences.real_recordable);

  // audience flags:
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_28k")),
			       preferences.real_audience & REAL_AUDIENCE_28_MODEM);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_56k")),
			       preferences.real_audience & REAL_AUDIENCE_56_MODEM);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_sisdn")),
			       preferences.real_audience & REAL_AUDIENCE_SINGLE_ISDN);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_disdn")),
			       preferences.real_audience & REAL_AUDIENCE_DUAL_ISDN);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_lan")),
			       preferences.real_audience & REAL_AUDIENCE_LAN_HIGH);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_dsl256")),
			       preferences.real_audience & REAL_AUDIENCE_256_DSL_CABLE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_dsl384")),
			       preferences.real_audience & REAL_AUDIENCE_384_DSL_CABLE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_real_audience_dsl512")),
			       preferences.real_audience & REAL_AUDIENCE_512_DSL_CABLE);


#else
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_server_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_infos_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_stream_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(preferences_window,"prefs_real_audience_frame"),FALSE);
#endif

}

void
BuildPrefsDisplayFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_display_period"),
			    preferences.display_period);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window,"prefs_display_keep_ratio")),
			       preferences.display_keep_ratio);
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

  if(stat("/dev/video1394",&statstruct)==0)
    // the device is there, check RW permissions
    if ((statstruct.st_mode&&S_IRUSR)&&(statstruct.st_mode&&S_IWUSR))
      video_ok=1;

  // BUILD A NEW  OPTION_MENU:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(preferences_window,"prefs_receive_method_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (preferences_window), "prefs_receive_method_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(preferences_window,"table45")),
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

  if (video_ok==1)
    {
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
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(preferences_window, "prefs_receive_method_menu")),
			      preferences.receive_method2index[preferences.receive_method]);

  gtk_entry_set_text(GTK_ENTRY(lookup_widget(preferences_window, "prefs_video1394_device")),
		     preferences.video1394_device);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(preferences_window, "prefs_receive_dropframes")),
		     preferences.video1394_dropframes);
}

void
BuildOptionFrame(void)
{
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(commander_window,
							  "mono16_bpp"),uiinfo->bpp);
  BuildBayerMenu();
  BuildBayerPatternMenu();
}
