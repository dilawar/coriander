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

#include "coriander.h"

extern char* format_list[5];
extern char* format0_list[7];
extern char* format1_list[8];
extern char* format2_list[8];
extern char* format6_list[1];
extern char* format7_list[8];

void UpdateFormatMenu(void)
{
  quadlet_t formats;

  if (dc1394_query_supported_formats(camera->camera_info.handle, camera->camera_info.id, &formats)!=DC1394_SUCCESS) {
    MainError("Could not query supported formats");
    formats=0x0;
  }

  // also disable 'window menu' items (this should go in another function, I know...)
  gtk_widget_set_sensitive(lookup_widget(main_window,"vbox34"),formats & (0x1<<24) );
  gtk_widget_set_sensitive(lookup_widget(main_window,"temperature_frame"),
			   camera->feature_set.feature[FEATURE_TEMPERATURE-FEATURE_MIN].available );
  
}



