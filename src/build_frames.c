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
}

void
BuildTriggerFrame(void)
{
  GtkAdjustment *adjustment;

  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"trigger_frame"),TRUE);

  BuildTriggerModeMenu();
  BuildFpsMenu();
  
  // set the trigger_count value adjustment
  adjustment=(GtkAdjustment*)gtk_adjustment_new(0,1,(int)0xFFF,1,10,0);// max. number for trigger parameter is 12bit=FFFh
  gtk_spin_button_set_adjustment((GtkSpinButton*)lookup_widget(commander_window, "trigger_count"),adjustment);
 
  // TODO: connect signal
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
  if (!err) MainError("Could not query basic functionalities");

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
  BuildFormat7Ranges();
  
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
}

void
BuildPrefsFtpFrame(void)
{
}

void
BuildPrefsRealFrame(void)
{
  GtkMenuItem* menuitem;
  GtkMenu* menu;
  GtkOptionMenu* option_menu;
  int i;

  option_menu=(GtkOptionMenu*)lookup_widget(preferences_window, "prefs_real_audience");
  menu=(GtkMenu*)gtk_option_menu_get_menu(option_menu);
  for (i=0;i<8;i++)
    {
      gtk_menu_set_active(menu, i);
      menuitem=(GtkMenuItem*)gtk_menu_get_active(menu);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_real_audience_activate),
			  (int*)i); 
    }

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

}

void
BuildPrefsDisplayFrame(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  int k=0;

  // BUILD A NEW  OPTION_MENU:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(preferences_window,"prefs_display_method_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (preferences_window), "prefs_display_method_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(preferences_window, "table44")),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // always add a GDK item
  glade_menuitem = gtk_menu_item_new_with_label (_("GDK"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_prefs_display_method_activate),
		      (int*)DISPLAY_METHOD_GDK); 
  //preferences.display_index2method[k]=DISPLAY_METHOD_GDK;
  preferences.display_method2index[DISPLAY_METHOD_GDK]=k;
  k++;

#ifdef HAVE_X11_EXTENSIONS_XVLIB_H
  // 'xv' menuitem optional addition:
  glade_menuitem = gtk_menu_item_new_with_label (_("Xv (single camera only)"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_prefs_display_method_activate),
		      (int*)DISPLAY_METHOD_XV); 
  //preferences.display_index2method[k]=DISPLAY_METHOD_XV;
  preferences.display_method2index[DISPLAY_METHOD_XV]=k;
  k++;
#endif

#ifdef HAVE_SDLLIB
  // 'sdl' menuitem optional addition:
  glade_menuitem = gtk_menu_item_new_with_label (_("SDL"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_prefs_display_method_activate),
		      (int*)DISPLAY_METHOD_SDL); 
  //preferences.display_index2method[k]=DISPLAY_METHOD_SDL;
  preferences.display_method2index[DISPLAY_METHOD_SDL]=k;
  k++;
#endif

  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

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
  //preferences.receive_index2method[k]=RECEIVE_METHOD_RAW1394;
  preferences.receive_method2index[RECEIVE_METHOD_RAW1394]=k;
  k++;

  //fprintf(stderr,"%d\n",camera_num);
  if (video_ok==1)
    {
      // 'xv' menuitem optional addition:
      glade_menuitem = gtk_menu_item_new_with_label (_("VIDEO1394"));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_prefs_receive_method_activate),
			  (int*)RECEIVE_METHOD_VIDEO1394); 
      //preferences.receive_index2method[k]=RECEIVE_METHOD_VIDEO1394;
      preferences.receive_method2index[RECEIVE_METHOD_VIDEO1394]=k;
      k++;
    }
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  //fprintf(stderr,"%d\n",camera_num);
}
