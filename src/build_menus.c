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
#include "definitions.h"
#include "callbacks.h"
#include "tools.h"
#include "build_menus.h"
#include <libdc1394/dc1394_control.h>

extern dc1394_feature_set *feature_set;
extern dc1394_miscinfo *misc_info;
extern GtkWidget *commander_window;
extern GtkWidget *format7_window;
extern char* fps_label_list[NUM_FRAMERATES];
extern char* format7_color_list[NUM_MODE_FORMAT7];
extern char* format7_mode_list[NUM_MODE_FORMAT7];
extern char* channel_num_list[16];
extern char* trigger_mode_list[4];
extern Format7Info *format7_info;
extern dc1394_camerainfo *camera;
extern dc1394_camerainfo *cameras;
extern int current_camera;
extern int camera_num;

void
BuildTriggerModeMenu(void)
{
  int err, i, f, modes;
  quadlet_t value;
  int k=0;
  int index[NUM_TRIGGER_MODE];

  GtkWidget* trigger_mode;
  GtkWidget* trigger_mode_menu;
  GtkWidget* glade_menuitem;

  err=dc1394_query_feature_characteristics(camera->handle,camera->id,FEATURE_TRIGGER,&value);
  if (!err) MainError("Could not query trigger feature characteristics");
  modes=( (value & (0xF << 12))>>12 );
  gtk_widget_destroy(GTK_WIDGET (lookup_widget(commander_window,"trigger_mode"))); // remove previous menu

  trigger_mode = gtk_option_menu_new ();
  gtk_widget_ref (trigger_mode);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "trigger_mode", trigger_mode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (trigger_mode);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(commander_window,"table17")), trigger_mode, 0, 2, 1, 2);
  gtk_container_set_border_width (GTK_CONTAINER (trigger_mode), 1);

  trigger_mode_menu = gtk_menu_new ();

  if (modes) // at least one mode present
    { // external trigger available:
      for (f=TRIGGER_MODE_MIN,i=0;f<=TRIGGER_MODE_MAX;i++,f++)
	{
	  if (modes & (0x1<<(TRIGGER_MODE_MAX-f)))
	    {
	      index[i]=k;
	      k++;
	      glade_menuitem = gtk_menu_item_new_with_label (_(trigger_mode_list[i]));
	      gtk_widget_show (glade_menuitem);
	      gtk_menu_append (GTK_MENU (trigger_mode_menu), glade_menuitem);
	      gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
				  GTK_SIGNAL_FUNC (on_trigger_mode_activate),
				  (int*)f);
	    }
	  else
	    index[i]=0;
	}
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (trigger_mode), trigger_mode_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (trigger_mode), index[feature_set->feature[FEATURE_TRIGGER-FEATURE_MIN].trigger_mode]);
}


void
BuildMemoryChannelMenu(void)
{
  int i;
  // note: this function does not require indexing the menu items for they are always in the same consecutive order
  GtkWidget* channel_num;
  GtkWidget* channel_num_menu;
  GtkWidget* glade_menuitem;

  misc_info->save_channel=misc_info->load_channel;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(commander_window,"memory_channel"))); // remove previous menu

  channel_num = gtk_option_menu_new ();
  gtk_widget_ref (channel_num);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "memory_channel", channel_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (channel_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(commander_window,"table16")), channel_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (channel_num), 1);

  channel_num_menu = gtk_menu_new ();

  for (i=0;i<=misc_info->mem_channel_number;i++)
    {
      glade_menuitem = gtk_menu_item_new_with_label (_(channel_num_list[i]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (channel_num_menu), glade_menuitem);
      gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_memory_channel_activate),
			  (int*)i); // i is an int passed in a pointer variable. This is 'normal'.
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (channel_num), channel_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (channel_num), misc_info->load_channel);
}

void
BuildCameraMenu(void)
{
  int i;
  // note: this function does not require indexing the menu items for they are always in the same consecutive order
  GtkWidget* camera_id;
  GtkWidget* camera_id_menu;
  GtkWidget* glade_menuitem;
  char tmp[STRING_SIZE];

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(commander_window,"camera_select"))); // remove previous menu

  camera_id = gtk_option_menu_new ();
  gtk_widget_ref (camera_id);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "camera_select", camera_id,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camera_id);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(commander_window,"table9")), camera_id, 1, 2, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (camera_id), 1);

  camera_id_menu = gtk_menu_new ();

  for (i=0;i<camera_num;i++)
    {
      sprintf(tmp,"Node %d: %s %s",cameras[i].id, cameras[i].vendor, cameras[i].model);
      glade_menuitem = gtk_menu_item_new_with_label (_(tmp));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (camera_id_menu), glade_menuitem);
      gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			  GTK_SIGNAL_FUNC (on_camera_select_activate),
			  (int*)i); // i is an int passed in a pointer variable. This is 'normal'.
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (camera_id), camera_id_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (camera_id) , current_camera);

}


void
BuildFormat7ModeMenu(void)
{
  int i,f;
  int k=0;
  int index[NUM_MODE_FORMAT7];

  GtkWidget* mode_num;
  GtkWidget* mode_num_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(format7_window,"format7_mode"))); // remove previous menu

  mode_num = gtk_option_menu_new ();
  gtk_widget_ref (mode_num);
  gtk_object_set_data_full (GTK_OBJECT (format7_window), "format7_mode", mode_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(format7_window,"table19")), mode_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (mode_num), 1);

  mode_num_menu = gtk_menu_new ();

  for (f=MODE_FORMAT7_MIN,i=0;f<=MODE_FORMAT7_MAX;f++,i++)
    {
      if (format7_info->mode[f-MODE_FORMAT7_MIN].present)
	{
	  index[i]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format7_mode_list[i]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_edit_format7_mode_activate),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[i]=0;
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (mode_num), mode_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_num), index[format7_info->edit_mode-MODE_FORMAT7_MIN]); // default: use the first mode for edit

}


void
BuildFormat7ColorMenu(void)
{
  int i, f;
  int k=0;
  int index[NUM_COLOR_FORMAT7];

  GtkWidget* color_num;
  GtkWidget* color_num_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(format7_window,"format7_color"))); // remove previous menu

  color_num = gtk_option_menu_new ();
  gtk_widget_ref (color_num);
  gtk_object_set_data_full (GTK_OBJECT (format7_window), "format7_color", color_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(format7_window,"table19")), color_num, 2, 3, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (color_num), 1);

  color_num_menu = gtk_menu_new ();

  for (f=COLOR_FORMAT7_MIN,i=0;f<=COLOR_FORMAT7_MAX;f++,i++)
    {
      if ((format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].color_coding) & (0x1 << (31-i)))
	{
	  index[i]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format7_color_list[i]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (color_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_edit_format7_color_activate),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[i]=0;
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (color_num), color_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (color_num),
			       index[format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].color_coding_id]);

}


void
BuildFpsMenu(void)
{
  int i, f, err;
  quadlet_t value;
  GtkWidget* fps;
  GtkWidget* fps_menu;
  GtkWidget* glade_menuitem;
  int index[NUM_FRAMERATES];
  int k=0;
  int new_framerate=0;
  dc1394bool_t cont=DC1394_TRUE;
  char temp[STRING_SIZE];

  if( misc_info->format == FORMAT_SCALABLE_IMAGE_SIZE)
  {
    value = 0; /* format 7 has no fixed framerates */
    gtk_widget_set_sensitive(lookup_widget(commander_window,"fps_menu"),FALSE);
  }
  else
  {
    gtk_widget_set_sensitive(lookup_widget(commander_window,"fps_menu"),TRUE);
    err=dc1394_query_supported_framerates(camera->handle, camera->id, misc_info->format, misc_info->mode, &value);
    if (!err) MainError("Could not query supported framerates");
  
 
  gtk_widget_destroy(GTK_WIDGET (lookup_widget(commander_window,"fps_menu"))); // remove previous menu

  fps = gtk_option_menu_new ();
  gtk_widget_ref (fps);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "fps_menu", fps,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fps);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(commander_window,"table17")), fps, 0, 2, 2, 3);
  gtk_container_set_border_width (GTK_CONTAINER (fps), 1);

  fps_menu = gtk_menu_new ();

  k=0;
  for (f=FRAMERATE_MIN,i=0;f<=FRAMERATE_MAX;i++,f++)
    {
      if  ( value & (0x1<< (31-i) ) ) //31 to 31-num_framerates 
	{
	  index[i]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(fps_label_list[i]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (fps_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_fps_activate),
			      (int*)f);
	}
      else
	index[i]=-1; // framerate not supported
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (fps), fps_menu);

  // here we set the sensitiveness, AFTER the 'gtk_option_menu_set_menu' command:
  //gtk_widget_set_sensitive (lookup_widget(commander_window,"fps_menu"),
  //			    !(GTK_TOGGLE_BUTTON (lookup_widget(commander_window,"trigger_external")))->active);

  // sets the active menu item:
  if (index[misc_info->framerate-FRAMERATE_MIN]<0) // previously selected framerate unsupported!!
    { // automatically switch to nearest fps available
      for (i=1;i<=((NUM_FRAMERATES>>1)+1);i++) // search radius is num_framerates/2 +1 for safe rounding
	{ 
	  if (((misc_info->framerate-FRAMERATE_MIN-i)>=0) && cont)
	    {
	      if (index[misc_info->framerate-FRAMERATE_MIN-i]>=0) // try down
		{ 
		  new_framerate=misc_info->framerate-i;
		  cont=DC1394_FALSE;
		}
	    }
	  if (((misc_info->framerate-FRAMERATE_MIN+i)<NUM_FRAMERATES) && cont)
	    {
	      if (index[misc_info->framerate-FRAMERATE_MIN+i]>=0) // try up
		{ 
		  new_framerate=misc_info->framerate+i;
		  cont=DC1394_FALSE;
		}
	    }
	}
      sprintf(temp,"Invalid framerate. Updating to nearest: %s",fps_label_list[new_framerate-FRAMERATE_MIN]);
      MainStatus(temp);
      err=dc1394_set_video_framerate(camera->handle,camera->id,new_framerate);
      if (!err) MainError("Cannot set video framerate");
      misc_info->framerate=new_framerate;
    }
  gtk_option_menu_set_history (GTK_OPTION_MENU (fps), index[misc_info->framerate-FRAMERATE_MIN]);
  }
}




