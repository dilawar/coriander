/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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
#include "build_ranges.h"
#include "definitions.h"
#include "tools.h"
#include "preferences.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *format7_window;
extern char* feature_scale_list[NUM_FEATURES];
extern char* feature_frame_list[NUM_FEATURES];
extern char* feature_menu_list[NUM_FEATURES];
extern char* feature_menu_table_list[NUM_FEATURES]; 
extern char* feature_menu_items_list[NUM_FEATURES];
extern Format7Info *format7_info;
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
  int stable, prec_value, prec_valuegoal, prec_valueBU, prec_valuecurrent, prec_valueRV;
  GtkAdjustment* adj;
  stable=0;
  err=0;
  prec_value=-1e7;// out of range data
  prec_valuegoal=-1e7;
  prec_valuecurrent=-1e7;
  prec_valueBU=-1e7;
  prec_valueRV=-1e7;

  // grab&set range value if readable:
  if (feature_set->feature[feature-FEATURE_MIN].readout_capable)
    { switch(feature)
      {
        case FEATURE_WHITE_BALANCE:
	  while(!stable)
	    {
	      err=dc1394_get_white_balance(camera->handle,camera->id,&valueBU,&valueRV);
	      if (((valueBU==prec_valueBU)&&(valueRV==prec_valueRV))||(err<0))
		stable=1;
	      else
		{
		  prec_valueBU=valueBU;
		  prec_valueRV=valueRV;
		  usleep(100000);// wait 1/10 sec
		}
	    }
	  if (err<0) MainError("Could not get white balance value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "white_balance_BU_scale")));
	      adj->value=valueBU;
	      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "white_balance_RV_scale")));
	      adj->value=valueRV;
	      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    }
	  break;
        case FEATURE_TEMPERATURE:
	  while(!stable)
	    {
	      err=dc1394_get_temperature(camera->handle,camera->id,&valuegoal,&valuecurrent);
	      if (((valuegoal==prec_valuegoal)&&(valuecurrent==prec_valuecurrent))||(err<0))
		stable=1;
	      else
		{
		  prec_valuegoal=valuegoal;
		  prec_valuecurrent=valuecurrent;
		  usleep(100000);// wait 1/10 sec
		}
	    }
	  if (err<0) MainError("Could not get temperature value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temperature_target_scale")));
	      adj->value=valuegoal;
	      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, "temperature_current_scale")));
	      adj->value=valuecurrent;
	      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	    }
	  break;
        default:
	  while(!stable)
	    {
	      err=dc1394_get_feature_value(camera->handle,camera->id,feature,&value);
	      if ((value==prec_value)||(err<0))
		stable=1;
	      else
		{
		  prec_value=value;
		  usleep(100000);// wait 1/10 sec
		}
	    }
	  if (err<0) MainError("Could not get feature value");
	  else
	    {
	      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, feature_scale_list[feature-FEATURE_MIN])));
	      adj->value=value;
	      gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
	      
	    }
	  break;
      }
    }

}

void
UpdateFormat7BppRange(void)
{
  GtkAdjustment* adj;
  Format7ModeInfo *info;
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];

  if (dc1394_query_format7_byte_per_packet(camera->handle,camera->id,format7_info->edit_mode,
					   &info->bpp)==DC1394_SUCCESS)
    if (dc1394_query_format7_packet_para(camera->handle,camera->id,format7_info->edit_mode,
					 &info->min_bpp,&info->max_bpp)==DC1394_SUCCESS)
      {
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_packet_size")));
	adj->upper=info->max_bpp;
	adj->lower=info->min_bpp;
	adj->value=info->bpp;
	adj->step_increment=1;
	adj->page_increment=(info->max_bpp-info->min_bpp)/16;
	gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");
      }
    else
      MainError("Can't get bpp info from camera");
  else
    MainError("Can't get bpp info from camera");
  //fprintf(stderr,"mode %d, bpp: %d, max: %d, min: %d\n",
  //format7_info->edit_mode,format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].bpp,
  //format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].max_bpp,
  //format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN].min_bpp);
}

void
UpdateFormat7Ranges(void)
{
  GtkAdjustment  *adj;
  Format7ModeInfo *info;
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];

  //fprintf(stderr,"size: %d %d\n",info->max_size_x,info->max_size_y);
  // define the adjustments for the 4 format7 controls. Note that (pos_x+size_x)<=max_size_x which yields some inter-dependencies

  // define adjustement for X-position
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hposition_scale")));
  adj->value=info->pos_x;
  adj->upper=info->max_size_x-info->size_x;
  adj->lower=0;
  if (info->use_unit_pos>0)
    adj->step_increment=info->step_pos_x;
  else
    adj->step_increment=info->step_x;
  adj->page_increment=adj->step_increment*4;
  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");

  // define adjustement for Y-position 
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vposition_scale")));
  adj->value=info->pos_y;
  adj->upper=info->max_size_y-info->size_y;
  adj->lower=0;
  if (info->use_unit_pos>0)
    adj->step_increment=info->step_pos_y;
  else
    adj->step_increment=info->step_y;
  adj->page_increment=adj->step_increment*4;
  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");

  // define adjustement for X-size
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_hsize_scale")));
  adj->value=info->size_x;
  adj->upper=info->max_size_x-info->pos_x;
  adj->lower=info->step_x;
  adj->page_increment=info->step_x*4;
  adj->step_increment=info->step_x;
  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");

  // define adjustement for X-size
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(format7_window, "format7_vsize_scale")));
  adj->value=info->size_y;
  adj->upper=info->max_size_y-info->pos_y;
  adj->lower=info->step_y;
  adj->page_increment=info->step_y*4;
  adj->step_increment=info->step_y;
  gtk_signal_emit_by_name(GTK_OBJECT (adj), "changed");

}

