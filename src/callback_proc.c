/*
 * Copyright (C) 2000-2001 Damien Douxchamps  <douxchamps@ieee.org>
 * Iso video receive, video overlay, and catpure provided by 
 * Dan Dennedy <dan@dennedy.org>
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
#include "callback_proc.h"
#include "support.h"
#include "update_ranges.h"
#include "definitions.h"
#include "tools.h"
#include "preferences.h"
#include "thread_display.h"
#include <libdc1394/dc1394_control.h>
#include <string.h>

extern GtkWidget *porthole_window;
extern char* feature_op_list[NUM_FEATURES];
extern char* feature_scale_list[NUM_FEATURES];
extern dc1394_camerainfo *camera;
extern dc1394_feature_set *feature_set;
extern dc1394_miscinfo *misc_info;
extern guint gIdleID;

extern unsigned char g_rgb_buffer[640*480*3];
extern dc1394_cameracapture g_single_capture;
extern gchar g_filename[256];
extern gchar g_ext[256];
extern PrefsInfo preferences; 

void OpRangeProcedure(GtkButton* widget, int feature)
{
  int err;
  float timeout_bin=0;
  float step;
  dc1394bool_t value=TRUE;

  step=1.0/preferences.auto_update_frequency;

  err=dc1394_start_one_push_operation(camera->handle, camera->id, feature);
  if (!err) MainError("Could not start one-push operation");
  else
    {
      gtk_widget_set_sensitive(GTK_WIDGET (widget), FALSE);
      while ( value && (timeout_bin<preferences.op_timeout) )
	{
	  sleep(step);
	  err=dc1394_is_one_push_in_operation(camera->handle, camera->id, feature, &value);
	  if (!err) MainError("Could not query one-push operation");
	  // the next line is not working: only the last update occurs (this is a Sony bug...)
	  if (preferences.auto_update)
	    UpdateRangeValue(GTK_WIDGET (widget),feature);
	  timeout_bin+=step;
	}
      if (timeout_bin>=preferences.op_timeout)
	MainStatus("One-Push function timed-out!\n");
      gtk_widget_set_sensitive(GTK_WIDGET (widget),TRUE);
    }
}

void ManRangeProcedure(GtkToggleButton *widget, int feature)
{
  int err;

  if (widget->active)
    UpdateRangeValue(GTK_WIDGET (widget),feature);

  // sensitive OP function if available and in man mode:
  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), feature_op_list[feature-FEATURE_MIN])),
			   feature_set->feature[feature-FEATURE_MIN].one_push && widget->active);

  // sensitive ranges accoring to auto/man mode and update value:
  switch (feature)
    {
      case FEATURE_WHITE_BALANCE:
	gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "whitebal_RV_scale")),widget->active);
	gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "whitebal_BU_scale")),widget->active);break;
      case FEATURE_TEMPERATURE:
	gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "temp_goal_scale")),widget->active);break;
	// only the goal range is active, the other one is just an indicator
      default:
	gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), feature_scale_list[feature-FEATURE_MIN])),widget->active);
    }

  err=dc1394_auto_on_off(camera->handle, camera->id, feature, !widget->active);
  if (!err) MainError("Could not set auto mode");
  else feature_set->feature[feature-FEATURE_MIN].auto_active=!widget->active;

}


void PowerRangeProcedure(GtkToggleButton* widget, int feature)
{
  int err;
  err=dc1394_feature_on_off(camera->handle, camera->id, feature, widget->active);
  if (!err) MainError("Could not set feature on/off");
  else
    {
      feature_set->feature[feature-FEATURE_MIN].is_on=widget->active;

      switch(feature)
	{
	case FEATURE_WHITE_BALANCE:
	  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "whitebal_RV_scale")),widget->active);
	  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "whitebal_BU_scale")),widget->active);break;
	case FEATURE_TEMPERATURE:
	  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), "temp_goal_scale")),widget->active);break;
	default:
	  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(GTK_WIDGET (widget), feature_scale_list[feature-FEATURE_MIN])),widget->active);
	}
    }
}
