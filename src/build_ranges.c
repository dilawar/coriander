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
#include "preferences.h"
#include "update_ranges.h"
#include "build_ranges.h"
#include <libdc1394/dc1394_control.h>


extern dc1394_feature_set *feature_set;
extern GtkWidget *format7_window;
extern GtkWidget *preferences_window;
extern Format7Info *format7_info;
extern char* feature_frame_list[NUM_FEATURES];
extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_menu_list[NUM_FEATURES];
extern char* feature_menu_table_list[NUM_FEATURES];
extern char* feature_menu_items_list[NUM_FEATURES];

void BuildRange(GtkWidget* current_window, int feature)
{
  GtkAdjustment *adjustment, *adjustment2;
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  // BUILD A NEW  OPTION_MENU:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(current_window,feature_menu_list[feature-FEATURE_MIN]))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (current_window), feature_menu_list[feature-FEATURE_MIN], new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(current_window, feature_menu_table_list[feature-FEATURE_MIN])),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();
  /*
  fprintf(stderr,"feature %d: avail: %d, onoff: %d, man: %d, auto: %d, op: %d\n",feature-FEATURE_MIN,
	  feature_set->feature[feature-FEATURE_MIN].available,
	  feature_set->feature[feature-FEATURE_MIN].on_off_capable,
	  feature_set->feature[feature-FEATURE_MIN].manual_capable,
	  feature_set->feature[feature-FEATURE_MIN].auto_capable,
	  feature_set->feature[feature-FEATURE_MIN].one_push);
  */
  if (feature_set->feature[feature-FEATURE_MIN].available)
    {
      // BUILD MENU ITEMS ====================================================================================
      // 'off' menuitem optional addition:
      if (feature_set->feature[feature-FEATURE_MIN].on_off_capable>0)
	{
	  glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_OFF]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_range_menu_activate),
			      (int*)(feature*1000+RANGE_MENU_OFF)); // i is an int passed in a pointer variable. This is 'normal'.
	}
      // 'man' menuitem optional addition:
      if (feature_set->feature[feature-FEATURE_MIN].manual_capable>0)
	{
	  glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_MAN]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_range_menu_activate),
			      (int*)(feature*1000+RANGE_MENU_MAN));
	}
      // 'auto' menuitem optional addition:
      if (feature_set->feature[feature-FEATURE_MIN].auto_capable>0)
	{
	  glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_AUTO]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_range_menu_activate),
			      (int*)(feature*1000+RANGE_MENU_AUTO));
	}
      // 'single' menuitem optional addition:
      if (feature_set->feature[feature-FEATURE_MIN].one_push>0)
	{
	  glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_SINGLE]));
	  gtk_widget_show (glade_menuitem);
	  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
	  gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			      GTK_SIGNAL_FUNC (on_range_menu_activate),
			      (int*)(feature*1000+RANGE_MENU_SINGLE));
	}

      gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

      // BUILD SCALE: ====================================================================================
      adjustment=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						    feature_set->feature[feature-FEATURE_MIN].min,
						    feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
      switch(feature)
	{
	  case FEATURE_WHITE_BALANCE:
	    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "white_balance_BU_scale"),adjustment);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "white_balance_RV_scale"),adjustment2);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed),
				(int*) FEATURE_WHITE_BALANCE+BU);
	    gtk_signal_connect (GTK_OBJECT (adjustment2), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed),
				(int*) FEATURE_WHITE_BALANCE+RV);
	    break;
	  case FEATURE_TEMPERATURE:
	    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "temperature_target_scale"),adjustment);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "temperature_current_scale"),adjustment2);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_TEMPERATURE);
	    break;
	  default:
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, feature_scale_list[feature-FEATURE_MIN]),adjustment);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) feature);

	}
    }
  else // feature not available
    {
      // BUILD DUMMY MENUTIEM:
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_NA]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      // note: no signal connected.

      gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);
    }
    
}

void
BuildFormat7Ranges(void)
{
  
  GtkAdjustment  *adjustment_px, *adjustment_py, *adjustment_sx, *adjustment_sy;
  Format7ModeInfo *info;
  
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];

  // define the adjustments for the 4 format7 controls. Note that (pos_x+size_x)<=max_size_x which yields some inter-dependencies

  // define adjustement for X-position
  adjustment_px=(GtkAdjustment*)gtk_adjustment_new(info->pos_x,0,info->max_size_x-info->size_x-1,1,info->step_x,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_hposition_scale"),adjustment_px);
  gtk_signal_connect(GTK_OBJECT (adjustment_px), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_POS_X);
  
  // define adjustement for Y-position 
  adjustment_py=(GtkAdjustment*)gtk_adjustment_new(info->pos_y,0,info->max_size_y-info->size_y-1,1,info->step_y,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_vposition_scale"),adjustment_py);
  gtk_signal_connect(GTK_OBJECT (adjustment_py), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_POS_Y);

  // define adjustement for X-size
  adjustment_sx=(GtkAdjustment*)gtk_adjustment_new(info->size_x,1,info->max_size_x-info->pos_x,1,info->step_x,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_hsize_scale"),adjustment_sx);
  gtk_signal_connect(GTK_OBJECT (adjustment_sx), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_SIZE_X);

  // define adjustement for X-size
  adjustment_sy=(GtkAdjustment*)gtk_adjustment_new(info->size_y,1,info->max_size_y-info->pos_y,1,info->step_y,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_vsize_scale"),adjustment_sy);
  gtk_signal_connect(GTK_OBJECT (adjustment_sy), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_SIZE_Y);

}
