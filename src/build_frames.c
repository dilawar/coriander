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


#include "build_frames.h"

extern GtkWidget *main_window;
extern GtkWidget *preferences_window;
extern GtkWidget *absolute_settings_window;
extern PrefsInfo preferences;
extern camera_t* camera;
extern BusInfo_t* businfo;

void
BuildCameraFrame(void)
{
  BuildCameraMenu();
}

void
BuildServiceFrame(void)
{
#ifdef HAVE_FTPLIB
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_ftp"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_ftp"),FALSE);
#endif
#ifdef HAVE_SDLLIB
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_display"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_display"),FALSE);
#endif
  // add V4L test here ??

}

void
BuildTriggerFrame(void)
{

  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_frame"),TRUE);

  BuildTriggerModeMenu();
  BuildFpsMenu();
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"trigger_external")),
			       camera->feature_set.feature[FEATURE_TRIGGER-FEATURE_MIN].is_on);
}


void
BuildPowerFrame(void)
{
  quadlet_t basic_funcs;
  // these two functions are always present:
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_reset"),TRUE);

  // activate if camera capable of power on/off:
  if (dc1394_query_basic_functionality(camera->camera_info.handle,camera->camera_info.id, &basic_funcs)!=DC1394_SUCCESS)
    MainError("Could not query basic functionalities");

  gtk_widget_set_sensitive(lookup_widget(main_window,"power_on"),(basic_funcs & 0x1<<16));
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_off"),(basic_funcs & 0x1<<16));

}


void
BuildMemoryFrame(void)
{
  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_frame"),TRUE);

  // activate the mem channel menu:
  BuildMemoryChannelMenu();

}

void
BuildIsoFrame(void)
{
  // TODO: only if ISO capable
  if (dc1394_get_iso_status(camera->camera_info.handle,camera->camera_info.id, &camera->misc_info.is_iso_on)!=DC1394_SUCCESS)
    MainError("Can't get ISO status");
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_start"),!camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_restart"),camera->misc_info.is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_stop"),camera->misc_info.is_iso_on);

}

void
BuildGlobalIsoFrame(void)
{
  // TODO: only if ISO capable
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_start"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_restart"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_stop"),TRUE);

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
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "prefs_save_period"), preferences.save_period);
  // scratch
  //fprintf(stderr,"building save scratch options. current: %d\n",preferences.save_scratch);
  switch(preferences.save_scratch) {
  case SAVE_SCRATCH_OVERWRITE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_scratch"),TRUE);
    break;
  case SAVE_SCRATCH_SEQUENTIAL:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_seq"),TRUE);
    break;
  case SAVE_SCRATCH_VIDEO:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_video"),TRUE);
    break;
  }
  // scratch
  if (preferences.save_convert == SAVE_CONVERT_ON)
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_convert"),TRUE);
  else
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_noconvert"),TRUE);
  
  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_save_filename")), preferences.save_filename);

  // file sequence tags
  if (preferences.save_datenum==SAVE_TAG_DATE)
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_date_tag"),TRUE);
  else
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_save_num_tag"),TRUE);

  // ram buffer
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "use_ram_buffer"),preferences.use_ram_buffer);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"ram_buffer_size"), preferences.ram_buffer_size);
  
}

void
BuildPrefsV4lFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,
							  "prefs_v4l_period"), preferences.v4l_period);
  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_v4l_dev_name")), preferences.v4l_dev_name);
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
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "prefs_ftp_period"), preferences.ftp_period);
  // scratch
  switch(preferences.ftp_scratch) {
  case FTP_SCRATCH_OVERWRITE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_scratch"),TRUE);
    break;
  case FTP_SCRATCH_SEQUENTIAL:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_seq"),TRUE);
    break;
  }
  // file,... names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_filename")), preferences.ftp_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_address")), preferences.ftp_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_password")),preferences.ftp_password);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_path")), preferences.ftp_path);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_user")), preferences.ftp_user);

#else

  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_scratch_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),FALSE);

#endif

  // file sequence tags
  if (preferences.ftp_datenum==FTP_TAG_DATE)
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_date_tag"),TRUE);
  else
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_num_tag"),TRUE);
}

void
BuildPrefsDisplayFrame(void)
{
  //GtkAdjustment* adj;
  //chain_t* service;
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"prefs_display_period"), preferences.display_period);

  // keep aspect ratio
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"prefs_display_keep_ratio")), preferences.display_keep_ratio);

  // display redraw
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"display_redraw")), preferences.display_redraw==DISPLAY_REDRAW_ON);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"display_redraw_rate"), preferences.display_redraw_rate);

  /*

  // display scaling
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"prefs_display_scale"), preferences.display_scale);
  
  adj=gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON (lookup_widget(main_window, "prefs_display_scale")));

  service=GetService(camera, SERVICE_DISPLAY);
  if (service==NULL) {
    adj->upper=10;
    adj->lower=0;
  }
  else {
    // do something smart here...
    adj->upper=10;
    adj->lower=0;
  }
  adj->step_increment=1;
  adj->page_increment=1;
  if (adj->value<adj->lower)
    adj->value=adj->lower;
  if (adj->value>adj->upper)
    adj->value=adj->upper;
  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
  
  */
}

void
BuildPrefsReceiveFrame(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  int k=0;

  // BUILD A NEW  OPTION_MENU:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"prefs_receive_method_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "prefs_receive_method_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table45")),
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

  // 'video1394' menuitem optional addition:
  glade_menuitem = gtk_menu_item_new_with_label (_("VIDEO1394"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_prefs_receive_method_activate),
		      (int*)RECEIVE_METHOD_VIDEO1394); 
  preferences.receive_method2index[RECEIVE_METHOD_VIDEO1394]=k;
  k++;
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "prefs_receive_method_menu")),
			      preferences.receive_method2index[preferences.receive_method]);

  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_video1394_device")), preferences.video1394_device);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window, "prefs_receive_dropframes")), preferences.video1394_dropframes);

  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "dma_buffer_size"), preferences.dma_buffer_size);
}

void
BuildOptionFrame(void)
{
  pthread_mutex_lock(&camera->uimutex);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "mono16_bpp"),camera->bpp);
  pthread_mutex_unlock(&camera->uimutex);
  BuildBayerMenu();
  BuildBayerPatternMenu();
  BuildStereoMenu();
}

void
BuildBandwidthFrame(void)
{
  GtkWidget *bandwidth_table;
  GtkWidget *label;
  GtkWidget *bandwidth_bar;
  char* temp;
  int nports, i;

  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // get the number of ports
  nports=businfo->port_num;

  //destroy table first.
  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window, "bandwidth_table")));

  // build new table
  bandwidth_table = gtk_table_new (nports, 5, TRUE);
  gtk_widget_ref (bandwidth_table);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "bandwidth_table", bandwidth_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bandwidth_table);
  gtk_container_add (GTK_CONTAINER (lookup_widget(main_window,"bandwidth_frame")), bandwidth_table);
  gtk_container_set_border_width (GTK_CONTAINER (bandwidth_table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (bandwidth_table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (bandwidth_table), 2);

  // build each bandwidth bar:
  for (i=0;i<nports;i++) {
    sprintf(temp,"Bus %d: ",i);
    label = gtk_label_new (_(temp));
    gtk_widget_ref (label);
    sprintf(temp,"label_bandwidth%d",i);
    gtk_object_set_data_full (GTK_OBJECT (main_window), temp, label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (bandwidth_table), label, 0, 1, i, i+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bandwidth_bar = gtk_progress_bar_new();
    gtk_widget_ref (bandwidth_bar);
    sprintf(temp,"bandwidth_bar%d",i);
    gtk_object_set_data_full (GTK_OBJECT (main_window), temp, bandwidth_bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bandwidth_bar);
    gtk_table_attach (GTK_TABLE (bandwidth_table), bandwidth_bar, 1, 5, i, i+1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_progress_set_show_text(GTK_PROGRESS (bandwidth_bar), 1);
    gtk_progress_set_text_alignment(GTK_PROGRESS (bandwidth_bar), .5, .5);
    gtk_progress_set_format_string(GTK_PROGRESS (bandwidth_bar),"%p %%");
  }

  free(temp);
}


