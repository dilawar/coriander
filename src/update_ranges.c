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
#include "update_ranges.h"
#include "definitions.h"
#include "tools.h"
#include <libdc1394/dc1394_control.h>

extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_man_list[NUM_FEATURES];
extern char* feature_auto_list[NUM_FEATURES];
extern char* feature_power_list[NUM_FEATURES];
extern GtkWidget *preferences_window;
extern dc1394_camerainfo *camera;
extern dc1394_feature_set *feature_set;
extern PrefsInfo preferences;


void
UpdateRange(GtkWidget* current_window, int feature)
{
  dc1394bool_t power_on, auto_mode;

  power_on=feature_set->feature[feature-FEATURE_MIN].is_on;
  // the power is on if:
  // 1.- the power is on (obvious!) or...
  // 2.- the feature cannot be powered on/off but is available (in which case it is supposed to be on)
  power_on=power_on || (!feature_set->feature[feature-FEATURE_MIN].on_off_capable && feature_set->feature[feature-FEATURE_MIN].available);
  auto_mode=feature_set->feature[feature-FEATURE_MIN].auto_active;

  // grab&set power status:
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(current_window, feature_power_list[feature-FEATURE_MIN]),
			       power_on );

  // grab&set auto/man status, plus sensitive range in man mode:
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(current_window, feature_auto_list[feature-FEATURE_MIN]),
			       power_on && auto_mode);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(current_window, feature_man_list[feature-FEATURE_MIN]),
			       power_on && !auto_mode);

  switch(feature)
    {
      case FEATURE_WHITE_BALANCE:
	gtk_widget_set_sensitive(lookup_widget(current_window, "whitebal_BU_scale"),(!auto_mode) && power_on);
	gtk_widget_set_sensitive(lookup_widget(current_window, "whitebal_RV_scale"),(!auto_mode) && power_on);break;
      case FEATURE_TEMPERATURE:
	// the only changeable range is the target one, the other is just an indicator.
	gtk_widget_set_sensitive(lookup_widget(current_window, "temp_goal_scale"),(!auto_mode) && power_on);break;
      default:
	gtk_widget_set_sensitive(lookup_widget(current_window, feature_scale_list[feature-FEATURE_MIN]), (!auto_mode) && power_on);
    }

  // grab&set range value if readable:
  UpdateRangeValue(current_window,feature);
}

void
UpdateRangeValue(GtkWidget* widget, int feature)
{
  // NOTE: range values are the only value the camera changes by itself. Contrary to other items
  //       (like auto/man or power), this item can thus change without the user's action. It is
  //       therefor mendatory to read the CAMERA value and not the value present in "feature_set".
  //       Moreover, we must WRITE the value read in the "feature_set" 'value' field.

  int  err, value, valueBU, valueRV, valuegoal, valuecurrent;
  GtkAdjustment* adj;

  // grab&set range value if readable:
  if (feature_set->feature[feature-FEATURE_MIN].readout_capable)
    { switch(feature)
      {
        case FEATURE_WHITE_BALANCE:
	  err=dc1394_get_white_balance(camera->handle,camera->id,&valueBU,&valueRV);
	  if (!err) MainError("Could not get white balance value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "whitebal_BU_scale")));
	      gtk_adjustment_set_value(adj, valueBU);
	      feature_set->feature[feature-FEATURE_MIN].BU_value=valueBU;
	      //printf("valueBU: %d\n",valueBU);
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "whitebal_RV_scale")));
	      gtk_adjustment_set_value(adj, valueRV);
	      //printf("valueRV: %d\n",valueRV);
	      feature_set->feature[feature-FEATURE_MIN].RV_value=valueRV;
	    }
	  break;
        case FEATURE_TEMPERATURE:
          err=dc1394_get_temperature(camera->handle,camera->id,&valuegoal,&valuecurrent);
	  if (!err) MainError("Could not get temperature value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temp_goal_scale")));
	      gtk_adjustment_set_value(adj, valuegoal);
	      feature_set->feature[feature-FEATURE_MIN].target_value=valuegoal;
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temp_current_scale")));
	      gtk_adjustment_set_value(adj, value);
	      feature_set->feature[feature-FEATURE_MIN].value=value;
	    }
	  break;
        default:
	  err=dc1394_get_feature_value(camera->handle,camera->id,feature,&value);
	  if (!err) MainError("Could not get feature value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, feature_scale_list[feature-FEATURE_MIN])));
	      gtk_adjustment_set_value(adj, value);
	      //printf("value: %d\n",value);
	      feature_set->feature[feature-FEATURE_MIN].value=value;
	    }
      }
    }

}


void
UpdatePrefsRanges(void)
{
  GtkAdjustment* adj;
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(preferences_window, "prefs_timeout_scale")));
  gtk_adjustment_set_value(adj, preferences.op_timeout);

  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(preferences_window, "prefs_update_scale")));
  gtk_adjustment_set_value(adj, preferences.auto_update_frequency);

  gtk_widget_set_sensitive(lookup_widget(preferences_window, "prefs_update_scale"),preferences.auto_update);

}
