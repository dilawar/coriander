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
#include "definitions.h"
#include "update_menus.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *commander_window;
extern dc1394_camerainfo *camera;
extern dc1394_feature_set *feature_set;
extern dc1394_miscinfo *misc_info;
extern char* format_list[5];
extern char* format0_list[7];
extern char* format1_list[8];
extern char* format2_list[8];
extern char* format6_list[1];
extern char* format7_list[8];

void UpdateFormatMenu(void)
{
  int err,i;
  quadlet_t modes, formats;

  err= dc1394_query_supported_formats(camera->handle, camera->id, &formats);
  if (!err)
    {
      MainError("Could not query supported formats");
      formats=0x0;
    }
  // also disable 'window menu' items (this should go in another function, I know...)
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format7_window"),formats & (0x1<<24) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format6_window"),formats & (0x1<<25) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"temp_frame"), feature_set->feature[FEATURE_TEMPERATURE-FEATURE_MIN].available );
  
  // formats capabilities:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_0"),formats & (0x1<<31) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_1"),formats & (0x1<<30) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_2"),formats & (0x1<<29) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_6"),formats & (0x1<<25) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_7"),formats & (0x1<<24) );
  
  // corresponding modes:

  // format 0:
  if (formats & (0x1<<31))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_VGA_NONCOMPRESSED, &modes);
      if (!err)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
      for (i=0;i<7;i++)
	{
	  gtk_widget_set_sensitive(lookup_widget(commander_window,format0_list[i]),modes & (0x1<<(31-i)) );
	  //if ( (modes & (0x1<<(31-i))) && (misc_info->mode==(MODE_FORMAT0_MIN+i)) && (misc_info->format==FORMAT_VGA_NONCOMPRESSED))
	  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (lookup_widget(commander_window,format0_list[i])), TRUE);
	}
    }

  // format 1:
  if (formats & (0x1<<30))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_1, &modes);
      if (!err)
	{
	  MainError("Could not query Format1 modes");
	  modes=0;
	}
      for (i=0;i<8;i++)
	{
	  gtk_widget_set_sensitive(lookup_widget(commander_window,format1_list[i]),modes & (0x1<<(31-i)) );
	  //if ( (modes & (0x1<<(31-i))) && (misc_info->mode==(MODE_FORMAT1_MIN+i)) && (misc_info->format==FORMAT_SVGA_NONCOMPRESSED_1))
	  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (lookup_widget(commander_window,format1_list[i])), TRUE);
	}
    }

  // format 2:
  if (formats & (0x1<<29))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_2, &modes);
      if (!err)
	{
	  MainError("Could not query Format2 modes");
	  modes=0;
	}
      for (i=0;i<8;i++)
	{
	  gtk_widget_set_sensitive(lookup_widget(commander_window,format2_list[i]),modes & (0x1<<(31-i)) );
	  //if ( (modes & (0x1<<(31-i))) && (misc_info->mode==(MODE_FORMAT2_MIN+i)) && (misc_info->format==FORMAT_SVGA_NONCOMPRESSED_2))
	  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (lookup_widget(commander_window,format2_list[i])), TRUE);
	}
    }

  // format 6:
  if (formats & (0x1<<25))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_STILL_IMAGE, &modes);
      if (!err)
	{
	  MainError("Could not query Format6 modes");
	  modes=0;
	}
      for (i=0;i<1;i++)
	{
	  gtk_widget_set_sensitive(lookup_widget(commander_window,format6_list[i]),modes & (0x1<<(31-i)) );
	  //if ( (modes & (0x1<<(31-i))) && (misc_info->mode==(MODE_FORMAT6_MIN+i)) && (misc_info->format==FORMAT_STILL_IMAGE))
	  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (lookup_widget(commander_window,format6_list[i])), TRUE);
	}
    }

  // format 7:
  if (formats & (0x1<<24))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SCALABLE_IMAGE_SIZE, &modes);
      if (!err)
	{
	  MainError("Could not query Format7 modes");
	  modes=0;
	}
      for (i=0;i<8;i++)
	{
	  gtk_widget_set_sensitive(lookup_widget(commander_window,format7_list[i]),modes & (0x1<<(31-i)) );
	  //if ( (modes & (0x1<<(31-i))) && (misc_info->mode==(MODE_FORMAT7_MIN+i)) && (misc_info->format==FORMAT_SCALABLE_IMAGE_SIZE))
	  //  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (lookup_widget(commander_window,format7_list[i])), TRUE);
	}
    }
}



