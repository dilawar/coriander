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
#include "preferences.h"
#include "tools.h"
#include "build_menus.h"
#include "conversions.h"
#include <libdc1394/dc1394_control.h>

extern dc1394_feature_set *feature_set;
extern dc1394_miscinfo *misc_info;
extern GtkWidget *commander_window;
extern GtkWidget *format7_window;
extern PrefsInfo preferences; 
extern char* fps_label_list[NUM_FRAMERATES];
extern char* format7_color_list[NUM_MODE_FORMAT7];
extern char* format7_mode_list[NUM_MODE_FORMAT7];
extern char* channel_num_list[16];
extern char* trigger_mode_list[4];
extern char* format0_list[NUM_FORMAT0_MODES];
extern char* format1_list[NUM_FORMAT1_MODES];
extern char* format2_list[NUM_FORMAT2_MODES];
extern char* format6_list[NUM_FORMAT6_MODES];
extern char* format7_list[NUM_MODE_FORMAT7];
extern Format7Info *format7_info;
extern dc1394_camerainfo *camera;
extern dc1394_camerainfo *cameras;
extern int current_camera;
extern int camera_num;
extern UIInfo *uiinfo;

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
  if (err<0) MainError("Could not query trigger feature characteristics");
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
      glade_menuitem = gtk_menu_item_new_with_label (_(preferences.camera_names[i]));
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
    if (err<0) MainError("Could not query supported framerates");
  
 
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
      if (err<0) MainError("Cannot set video framerate");
      misc_info->framerate=new_framerate;
    }
  gtk_option_menu_set_history (GTK_OPTION_MENU (fps), index[misc_info->framerate-FRAMERATE_MIN]);
  }
}


void
BuildFormatMenu(void)
{
  int f;
  int k=0;
  int index[2048];
  quadlet_t modes, formats;


  GtkWidget* mode_num;
  GtkWidget* mode_num_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(commander_window,"format_select"))); // remove previous menu

  mode_num = gtk_option_menu_new ();
  gtk_widget_ref (mode_num);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "format_select", mode_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(commander_window,"table60")), mode_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (mode_num), 1);

  mode_num_menu = gtk_menu_new ();

  // get supported formats
  if (dc1394_query_supported_formats(camera->handle, camera->id, &formats)<0)
    {
      MainError("Could not query supported formats");
      formats=0x0;
    }

  // FORMAT_0 -----------------------------------
  if (formats & (0x1<<31))
    {
      if (dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_VGA_NONCOMPRESSED, &modes)<0)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
    }
  else
    modes=0;

  for (f=MODE_FORMAT0_MIN;f<=MODE_FORMAT0_MAX;f++)
    {
      if (modes & (0x1<<(31-(f-MODE_FORMAT0_MIN))))
	{
	  index[f]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format0_list[f-MODE_FORMAT0_MIN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (ChangeModeAndFormat),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[f]=0;
    }

  // FORMAT_1 -----------------------------------
  if (formats & (0x1<<30))
    {
      if (dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_1, &modes)<0)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
    }
  else
    modes=0;

  for (f=MODE_FORMAT1_MIN;f<=MODE_FORMAT1_MAX;f++)
    {
      if (modes & (0x1<<(31-(f-MODE_FORMAT1_MIN))))
	{
	  index[f]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format1_list[f-MODE_FORMAT1_MIN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (ChangeModeAndFormat),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[f]=0;
    }

  // FORMAT_2 -----------------------------------
  if (formats & (0x1<<29))
    {
      if (dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_2, &modes)<0)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
    }
  else
    modes=0;

  for (f=MODE_FORMAT2_MIN;f<=MODE_FORMAT2_MAX;f++)
    {
      if (modes & (0x1<<(31-(f-MODE_FORMAT2_MIN))))
	{
	  index[f]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format2_list[f-MODE_FORMAT2_MIN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (ChangeModeAndFormat),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[f]=0;
    }

  // FORMAT_6 -----------------------------------
  if (formats & (0x1<<25))
    {
      if (dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_STILL_IMAGE, &modes)<0)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
    }
  else
    modes=0;

  for (f=MODE_FORMAT6_MIN;f<=MODE_FORMAT6_MAX;f++)
    {
      if (modes & (0x1<<(31-(f-MODE_FORMAT6_MIN))))
	{
	  index[f]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format6_list[f-MODE_FORMAT6_MIN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (ChangeModeAndFormat),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[f]=0;
    }

  // FORMAT_7 -----------------------------------
  if (formats & (0x1<<24))
    {
      if (dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SCALABLE_IMAGE_SIZE, &modes)<0)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
    }
  else
    modes=0;

  for (f=MODE_FORMAT7_MIN;f<=MODE_FORMAT7_MAX;f++)
    {
      if (modes & (0x1<<(31-(f-MODE_FORMAT7_MIN))))
	{
	  index[f]=k;
	  k++;
	  glade_menuitem = gtk_menu_item_new_with_label (_(format7_list[f-MODE_FORMAT7_MIN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (ChangeModeAndFormat),
			      (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
	}
      else
	index[f]=0;
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (mode_num), mode_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_num), index[misc_info->mode]);

}


void
BuildBayerMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(commander_window,"bayer_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "bayer_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,"table61")),
		    new_option_menu, 0, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // add no bayer option
  glade_menuitem = gtk_menu_item_new_with_label (_("No Bayer"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_menu_activate),
		      (int*)NO_BAYER_DECODING); 
  // add nearest_neighbor option
  glade_menuitem = gtk_menu_item_new_with_label (_("Nearest"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_menu_activate),
		      (int*)BAYER_DECODING_NEAREST); 
  // add EDGE_SENSE option
  glade_menuitem = gtk_menu_item_new_with_label (_("Edge Sense"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_menu_activate),
		      (int*)BAYER_DECODING_EDGE_SENSE); 
  
  // add DOWNSAMPLE option
  glade_menuitem = gtk_menu_item_new_with_label (_("Downsample"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_menu_activate),
		      (int*)BAYER_DECODING_DOWNSAMPLE); 
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(commander_window, "bayer_menu")),uiinfo->bayer);
      
}

void
BuildBayerPatternMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(commander_window,"pattern_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), "pattern_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,"table61")),
		    new_option_menu, 0, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // add no bayer option
  glade_menuitem = gtk_menu_item_new_with_label (_("BGGR"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_pattern_menu_activate),
		      (int*)BAYER_PATTERN_BGGR); 
  // add nearest_neighbor option
  glade_menuitem = gtk_menu_item_new_with_label (_("GRBG"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
		      GTK_SIGNAL_FUNC (on_bayer_pattern_menu_activate),
		      (int*)BAYER_PATTERN_GRBG); 

  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(commander_window, "pattern_menu")),
			      uiinfo->bayer_pattern);
      
}
