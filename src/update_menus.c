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

void UpdateFormatMenu(void)
{
  int err;
  quadlet_t modes, formats;

  // disable things that are not ready yet:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"load_setup"), 0  );// it's not ready yet.
  gtk_widget_set_sensitive(lookup_widget(commander_window,"save_setup"), 0  );// it's not ready yet.
  gtk_widget_set_sensitive(lookup_widget(commander_window,"save_setup_as"), 0  );// it's not ready yet.
  gtk_widget_set_sensitive(lookup_widget(commander_window,"preferences"), 0  );// it's not ready yet.

  err= dc1394_query_supported_formats(camera->handle, camera->id, &formats);
  if (!err)
    {
      MainError("Could not query supported formats");
      formats=0x0;
    }
  // also disable 'window menu' items (this should go in another function, I know...)
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format7_window"),formats & (0x1<<24) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format6_window"),formats & (0x1<<25) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"temp_window"), feature_set->feature[FEATURE_TEMPERATURE-FEATURE_MIN].available );
  
  // formats capabilities:
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_0"),formats & (0x1<<31) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_1"),formats & (0x1<<30) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_2"),formats & (0x1<<29) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_6"),formats & (0x1<<25) );
  gtk_widget_set_sensitive(lookup_widget(commander_window,"format_7"),formats & (0x1<<24) );
  
  // corresponding modes:
  if (formats & (0x1<<31))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_VGA_NONCOMPRESSED, &modes);
      if (!err)
	{
	  MainError("Could not query Format0 modes");
	  modes=0;
	}
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode0"),modes & (0x1<<31) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode1"),modes & (0x1<<30) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode2"),modes & (0x1<<29) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode3"),modes & (0x1<<28) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode4"),modes & (0x1<<27) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode5"),modes & (0x1<<26) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f0_mode6"),modes & (0x1<<25) );
    }

  if (formats & (0x1<<30))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_1, &modes);
      if (!err) MainError("Could not query Format1 modes");
      {
	MainError("Could not query Format1 modes");
	modes=0;
      }
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode0"),modes & (0x1<<31) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode1"),modes & (0x1<<30) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode2"),modes & (0x1<<29) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode3"),modes & (0x1<<28) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode4"),modes & (0x1<<27) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode5"),modes & (0x1<<26) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode6"),modes & (0x1<<25) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f1_mode7"),modes & (0x1<<24) );
    }

  if (formats & (0x1<<29))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SVGA_NONCOMPRESSED_2, &modes);
      if (!err)
	{
	  MainError("Could not query Format2 modes");
	  modes=0;
	}
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode0"),modes & (0x1<<31) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode1"),modes & (0x1<<30) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode2"),modes & (0x1<<29) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode3"),modes & (0x1<<28) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode4"),modes & (0x1<<27) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode5"),modes & (0x1<<26) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode6"),modes & (0x1<<25) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f2_mode7"),modes & (0x1<<24) );
    }

  if (formats & (0x1<<25))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_STILL_IMAGE, &modes);
      if (!err)
	{
	  MainError("Could not query Format6 modes");
	  modes=0;
	}
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f6_mode0"),modes & (0x1<<31) );
    }

  if (formats & (0x1<<24))
    {
      err= dc1394_query_supported_modes(camera->handle, camera->id, FORMAT_SCALABLE_IMAGE_SIZE, &modes);
      if (!err)
	{
	  MainError("Could not query Format7 modes");
	  modes=0;
	}
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode0"),modes & (0x1<<31) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode1"),modes & (0x1<<30) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode2"),modes & (0x1<<29) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode3"),modes & (0x1<<28) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode4"),modes & (0x1<<27) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode5"),modes & (0x1<<26) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode6"),modes & (0x1<<25) );
      gtk_widget_set_sensitive(lookup_widget(commander_window,"f7_mode7"),modes & (0x1<<24) );
    }
}



