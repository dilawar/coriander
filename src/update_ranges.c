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
#include "update_ranges.h"
#include "definitions.h"
#include "tools.h"
#include "preferences.h"
#include <libdc1394/dc1394_control.h>

extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_frame_list[NUM_FEATURES];
extern char* feature_menu_list[NUM_FEATURES];
extern char* feature_menu_table_list[NUM_FEATURES]; 
extern char* feature_menu_items_list[NUM_FEATURES];
extern GtkWidget *preferences_window;
extern dc1394_camerainfo *camera;
extern dc1394_feature_set *feature_set;
extern PrefsInfo preferences;


void
UpdateRange(GtkWidget* current_window, int feature)
{
  int index;

  // is feature available?
  if (feature_set->feature[feature-FEATURE_MIN].available==0)
    { // feature not available: unsensitive the frame
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_frame_list[feature-FEATURE_MIN]),FALSE);
    }
  else
    { // sensitive the current frame:
      gtk_widget_set_sensitive(lookup_widget(current_window, feature_frame_list[feature-FEATURE_MIN]),TRUE);

      // select the current menuitem:
      if ((!feature_set->feature[feature-FEATURE_MIN].is_on)&& // off
	  (feature_set->feature[feature-FEATURE_MIN].on_off_capable))
	{
	  index=0;
	}
      else
	if ((!feature_set->feature[feature-FEATURE_MIN].auto_active)&& // man
	    (feature_set->feature[feature-FEATURE_MIN].manual_capable))
	  {
	    index=1*(feature_set->feature[feature-FEATURE_MIN].on_off_capable>0);
	  }
        else
	  if ((feature_set->feature[feature-FEATURE_MIN].auto_active)&& // auto
	      (feature_set->feature[feature-FEATURE_MIN].auto_capable))
	    {
	      index=1*(feature_set->feature[feature-FEATURE_MIN].on_off_capable)+
		1*(feature_set->feature[feature-FEATURE_MIN].manual_capable);
	    }
	  else
	    {
	      index=1*(feature_set->feature[feature-FEATURE_MIN].on_off_capable)+// single
		1*(feature_set->feature[feature-FEATURE_MIN].manual_capable)+
		1*(feature_set->feature[feature-FEATURE_MIN].auto_capable);
	    }
      
      // sets the active menu item:
      gtk_option_menu_set_history (GTK_OPTION_MENU (lookup_widget(current_window,feature_menu_list[feature-FEATURE_MIN])), index);
    

      switch(feature)
	{
	case FEATURE_WHITE_BALANCE:
	  gtk_widget_set_sensitive(lookup_widget(current_window, "white_balance_BU_scale"),
				   (!feature_set->feature[feature-FEATURE_MIN].auto_active)&&
				   (!feature_set->feature[feature-FEATURE_MIN].one_push_active)&&
				   (!(feature_set->feature[feature-FEATURE_MIN].on_off_capable&&
				      !feature_set->feature[feature-FEATURE_MIN].is_on)));
	  gtk_widget_set_sensitive(lookup_widget(current_window, "white_balance_RV_scale"),
				   (!feature_set->feature[feature-FEATURE_MIN].auto_active)&&
				   (!feature_set->feature[feature-FEATURE_MIN].one_push_active)&&
				   (!(feature_set->feature[feature-FEATURE_MIN].on_off_capable&&
				      !feature_set->feature[feature-FEATURE_MIN].is_on)));break;
	case FEATURE_TEMPERATURE:
	  // the only changeable range is the target one, the other is just an indicator.
	  gtk_widget_set_sensitive(lookup_widget(current_window, "temperature_target_scale"),
				   (!feature_set->feature[feature-FEATURE_MIN].auto_active)&&
				   (!feature_set->feature[feature-FEATURE_MIN].one_push_active)&&
				   (!(feature_set->feature[feature-FEATURE_MIN].on_off_capable&&
				      !feature_set->feature[feature-FEATURE_MIN].is_on)));
	  gtk_widget_set_sensitive(lookup_widget(current_window, "temperature_current_scale"),FALSE);break;
	default:
	  gtk_widget_set_sensitive(lookup_widget(current_window, feature_scale_list[feature-FEATURE_MIN]),
				   (!feature_set->feature[feature-FEATURE_MIN].auto_active)&&
				   (!feature_set->feature[feature-FEATURE_MIN].one_push_active)&&
				   (!(feature_set->feature[feature-FEATURE_MIN].on_off_capable&&
				      !feature_set->feature[feature-FEATURE_MIN].is_on)));break;
	}

      // grab&set range value if readable:
      UpdateRangeValue(current_window,feature);
    }
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
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "white_balance_BU_scale")));
	      gtk_adjustment_set_value(adj, valueBU);
	      feature_set->feature[feature-FEATURE_MIN].BU_value=valueBU;
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "white_balance_RV_scale")));
	      gtk_adjustment_set_value(adj, valueRV);
	      feature_set->feature[feature-FEATURE_MIN].RV_value=valueRV;
	    }
	  break;
        case FEATURE_TEMPERATURE:
          err=dc1394_get_temperature(camera->handle,camera->id,&valuegoal,&valuecurrent);
	  if (!err) MainError("Could not get temperature value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temperature_target_scale")));
	      gtk_adjustment_set_value(adj, valuegoal);
	      feature_set->feature[feature-FEATURE_MIN].target_value=valuegoal;
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temperature_current_scale")));
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
	      feature_set->feature[feature-FEATURE_MIN].value=value;
	    }
	  break;
      }
    }

}
