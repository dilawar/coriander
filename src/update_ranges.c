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

void
UpdateRange(int feature)
{
  int index,i;
  char *stemp;
  dc1394bool_t range_is_active, abs_is_on;

  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

  int auto_capable=0;
  int manual_capable=0;
  int one_push_capable=0;

  for (i=0;i<camera->feature_set.feature[feature-DC1394_FEATURE_MIN].modes.num;i++) {
    switch (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].modes.modes[i]) {
    case DC1394_FEATURE_MODE_MANUAL:
      manual_capable=1;
      break;
    case DC1394_FEATURE_MODE_AUTO:
      auto_capable=1;
      break;
    case DC1394_FEATURE_MODE_ONE_PUSH_AUTO:
      one_push_capable=1;
      break;
    }
  }

  // select the current menuitem:
  if ((!camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on)&& // off
      (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable)) {
    index=0;
  }
  else {
    if ((camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode==DC1394_FEATURE_MODE_MANUAL)&& // man
	(manual_capable)&&
	(!(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable&& // abs control not on
	   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].abs_control))){
      index=1*(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable>0);
    }
    else {
      if ((camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode==DC1394_FEATURE_MODE_AUTO)&& // auto
	  (auto_capable)&&
	  (!(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable&& // abs control not on
	     camera->feature_set.feature[feature-DC1394_FEATURE_MIN].abs_control))){
	index=1*(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable)+
  	      1*(manual_capable);
      }
      else {
	if (!(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable&& // abs control not on
	   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].abs_control)) // single
	  index=1*(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable)+
	        1*(manual_capable)+
	        1*(auto_capable);
	else {
	  index=1*(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable)+ // absolute
	        1*(manual_capable)+
	        1*(auto_capable)+
	        1*(one_push_capable);
	  
	}
      }
    }
  }

  // sets the active menu item:
  sprintf(stemp,"feature_%d_menu",feature);
  gtk_option_menu_set_history (GTK_OPTION_MENU (lookup_widget(main_window,stemp)), index);
  
  range_is_active=((!(auto_capable && (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode==DC1394_FEATURE_MODE_AUTO)))&& // auto not on
		   (!(one_push_capable&& (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode==DC1394_FEATURE_MODE_ONE_PUSH_AUTO)))&& // one push not on
		   (!(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable&& // abs control not on
		      camera->feature_set.feature[feature-DC1394_FEATURE_MIN].abs_control))&&
		   (!(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable&& // feature is on
		      !camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on)));
  /*
  fprintf(stderr,"%d : %d %d %d %d %d : r=%d\n",
	  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable,
	  feature,manual_capable,auto_capable, one_push_capable,
	  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable,
	  range_is_active);
  */

  sprintf(stemp,"feature_%d_label",feature);
  gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);

  switch(feature) {
  case DC1394_FEATURE_WHITE_BALANCE:
    sprintf(stemp,"feature_%d_bu_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_rv_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_bu_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_rv_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    break;
  case DC1394_FEATURE_WHITE_SHADING:
    sprintf(stemp,"feature_%d_r_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_g_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_b_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_r_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_g_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_b_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    break;
  case DC1394_FEATURE_TEMPERATURE:
    // the only changeable range is the target one, the other is just an indicator.
    sprintf(stemp,"feature_%d_target_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_current_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp),FALSE);
    sprintf(stemp,"feature_%d_target_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_current_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp),FALSE);
    break;
  default:
    sprintf(stemp,"feature_%d_scale",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    sprintf(stemp,"feature_%d_spin",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), range_is_active);
    break;
  }
  // all features: set absolute range sensitivity:
  abs_is_on=((camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable&& // abs control is on
	      camera->feature_set.feature[feature-DC1394_FEATURE_MIN].abs_control) &&
	      camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on);
  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable) {
    sprintf(stemp,"feature_%d_abs_entry",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), abs_is_on);
    sprintf(stemp,"feature_%d_abs_label",feature);
    gtk_widget_set_sensitive(lookup_widget(main_window, stemp), abs_is_on);
  }
  // grab&set range value if readable:
  UpdateRangeValue(main_window,feature);

  free(stemp);
}

void
UpdateRangeValue(GtkWidget* widget, int feature)
{
  // NOTE: range values are the only value the camera changes by itself. Contrary to other items
  //       (like auto/man or power), this item can thus change without the user's action. It is
  //       therefor mendatory to read the CAMERA value and not the value present in "feature_set".
  //       Moreover, we must WRITE the value read in the "feature_set" 'value' field.

  int  err;
  unsigned int value, valueBU, valueRV, valuegoal, valuecurrent, valueR, valueG, valueB;
  char *stemp;
  GtkAdjustment* adj;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

  err=0;
  // grab&set range value if readable:
  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].readout_capable) {
    switch(feature) {
    case DC1394_FEATURE_WHITE_BALANCE:
	err=dc1394_feature_whitebalance_get_value(camera->camera_info,&valueBU,&valueRV);
      if (err!=DC1394_SUCCESS)
	Error("Could not get white balance value");
      else {
	sprintf(stemp,"feature_%d_bu_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valueBU;
	g_signal_emit_by_name((gpointer) adj, "changed");
	sprintf(stemp,"feature_%d_rv_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valueRV;
	g_signal_emit_by_name((gpointer) adj, "changed");
	/*
        // FIXME: spin buttons don't update
	sprintf(stemp,"feature_%d_bu_spin",feature);
	adj=gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON (lookup_widget(widget, stemp)));
	adj->value=valueBU;
	g_signal_emit_by_name((gpointer) adj, "changed");
	sprintf(stemp,"feature_%d_rv_spin",feature);
	adj=gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON (lookup_widget(widget, stemp)));
	adj->value=valueRV;
	g_signal_emit_by_name((gpointer) adj, "changed");
	*/
      }
      break;
    case DC1394_FEATURE_WHITE_SHADING:
	err=dc1394_feature_whiteshading_get_value(camera->camera_info,&valueR,&valueG, &valueB);
      if (err!=DC1394_SUCCESS)
	Error("Could not get white balance value");
      else {
	sprintf(stemp,"feature_%d_r_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valueR;
	g_signal_emit_by_name((gpointer) adj, "changed");
	sprintf(stemp,"feature_%d_g_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valueG;
	g_signal_emit_by_name((gpointer) adj, "changed");
	sprintf(stemp,"feature_%d_b_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valueB;
	g_signal_emit_by_name((gpointer) adj, "changed");
      }
      break;
    case DC1394_FEATURE_TEMPERATURE:
	err=dc1394_feature_temperature_get_value(camera->camera_info, &valuegoal, &valuecurrent);
      if (err<0)
	Error("Could not get temperature value");
      else {
	sprintf(stemp,"feature_%d_target_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget,stemp)));
	adj->value=valuegoal;
	g_signal_emit_by_name((gpointer) adj, "changed");
	sprintf(stemp,"feature_%d_current_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=valuecurrent;
	g_signal_emit_by_name((gpointer) adj, "changed");
      }
      break;
    default:
	err=dc1394_feature_get_value(camera->camera_info,feature,&value);
      if (err<0)
	  Error("Could not get feature value");
      else {
	sprintf(stemp,"feature_%d_scale",feature);
	adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(widget, stemp)));
	adj->value=value;
	g_signal_emit_by_name((gpointer) adj, "changed");
      }
      break;
    }
    if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable) {
      GetAbsValue(feature);
    }
  }
  
  free(stemp);
}

void
UpdateFormat7PacketSizeRange(void)
{
  GtkAdjustment* adj;
  dc1394format7mode_t *info;
  info=&camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];

  if (dc1394_format7_get_packet_size(camera->camera_info,
				     camera->format7_info.edit_mode, &info->packet_size)==DC1394_SUCCESS) {
    if (dc1394_format7_get_packet_parameters(camera->camera_info,
				       camera->format7_info.edit_mode, &info->unit_packet_size,&info->max_packet_size)==DC1394_SUCCESS) {
      if (info->packet_size>info->max_packet_size)
	info->packet_size=info->max_packet_size;
      //if (info->packet_size==0)
      //  fprintf(stderr,"PACKET_SIZE is zero in %s at line %d\n",__FUNCTION__,__LINE__);
      adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_packet_size")));
      adj->upper=info->max_packet_size;
      adj->lower=info->unit_packet_size;
      adj->value=info->packet_size;
      adj->step_increment=info->unit_packet_size;
      adj->page_increment=(info->max_packet_size-info->unit_packet_size)/16;
      g_signal_emit_by_name((gpointer) adj, "changed");
    }
    else
      Error("Can't get packet_size info from camera");
  }
  else
    Error("Can't get packet_size info from camera");
}

void
UpdateFormat7Ranges(void)
{
  GtkAdjustment  *adj;
  dc1394format7mode_t *info;
  info=&camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];

  // define the adjustments for the 4 format7 controls. Note that (pos_x+size_x)<=max_size_x which yields some inter-dependencies

  // define adjustement for X-position
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_hposition_scale")));
  adj->value=info->pos_x;
  adj->upper=info->max_size_x-info->size_x;
  adj->lower=0;
  adj->step_increment=info->unit_pos_x;
  adj->page_increment=adj->step_increment*4;
  g_signal_emit_by_name((gpointer) adj, "changed");

  // define adjustement for Y-position 
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_vposition_scale")));
  adj->value=info->pos_y;
  adj->upper=info->max_size_y-info->size_y;
  adj->lower=0;
  adj->step_increment=info->unit_pos_y;
  adj->page_increment=adj->step_increment*4;
  g_signal_emit_by_name((gpointer) adj, "changed");

  // define adjustement for X-size
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_hsize_scale")));
  adj->value=info->size_x;
  adj->upper=info->max_size_x-info->pos_x;
  adj->lower=info->unit_size_x;
  adj->step_increment=info->unit_size_x;
  adj->page_increment=adj->step_increment*4;
  g_signal_emit_by_name((gpointer) adj, "changed");

  // define adjustement for Y-size
  adj=gtk_range_get_adjustment(GTK_RANGE (lookup_widget(main_window, "format7_vsize_scale")));
  adj->value=info->size_y;
  adj->upper=info->max_size_y-info->pos_y;
  adj->lower=info->unit_size_y;
  adj->step_increment=info->unit_size_y;
  adj->page_increment=adj->step_increment*4;
  g_signal_emit_by_name((gpointer) adj, "changed");

}

