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
#include "update_ranges.h"
#include "build_ranges.h"
#include <libdc1394/dc1394_control.h>


extern dc1394_feature_set *feature_set;
extern GtkWidget *format7_window;
extern Format7Info *format7_info;
extern char* feature_op_list[NUM_FEATURES];
extern char* feature_frame_list[NUM_FEATURES];
extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_man_list[NUM_FEATURES];
extern char* feature_auto_list[NUM_FEATURES];
extern char* feature_power_list[NUM_FEATURES];

void BuildRange(GtkWidget* current_window, int feature)
{
  GtkAdjustment *adjustment, *adjustment2;

  if (feature_set->feature[feature-FEATURE_MIN].available)
    {
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_frame_list[feature-FEATURE_MIN]),TRUE);
      // one push auto:
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_op_list[feature-FEATURE_MIN]),
			       feature_set->feature[feature-FEATURE_MIN].one_push);
      // on/off switching:
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_power_list[feature-FEATURE_MIN]),
			       feature_set->feature[feature-FEATURE_MIN].on_off_capable);
      // manual mode:
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_man_list[feature-FEATURE_MIN]),
			       feature_set->feature[feature-FEATURE_MIN].manual_capable);
      // auto mode:
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_auto_list[feature-FEATURE_MIN]),
			       feature_set->feature[feature-FEATURE_MIN].auto_capable);

      // values:
      adjustment=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						    feature_set->feature[feature-FEATURE_MIN].min,
						    feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
      switch(feature)
	{
	  case FEATURE_WHITE_BALANCE:
	    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "whitebal_BU_scale"),adjustment);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "whitebal_RV_scale"),adjustment2);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_WHITE_BALANCE+BU);
	    gtk_signal_connect (GTK_OBJECT (adjustment2), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_WHITE_BALANCE+RV);
	    break;
	  case FEATURE_TEMPERATURE:
	    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].min,
							   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "temp_goal_scale"),adjustment);
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, "temp_current_scale"),adjustment2);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_TEMPERATURE);
	    break;
	  default:
	    gtk_range_set_adjustment((GtkRange*)lookup_widget(current_window, feature_scale_list[feature-FEATURE_MIN]),adjustment);
	    // connect:
	    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) feature);

	}
    }
    
}

void
BuildFormat7Ranges(void)
{
  
  GtkAdjustment  *adjustment_px, *adjustment_py, *adjustment_sx, *adjustment_sy;
  Format7ModeInfo *info;
  
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];
  printf( "BuildFormat7Ranges()\n");

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
