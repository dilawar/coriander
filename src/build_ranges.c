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

#include "build_ranges.h"

extern dc1394_feature_set *feature_set;
extern GtkWidget *format7_window;
extern GtkWidget *preferences_window;
extern GtkWidget *commander_window;
extern Format7Info *format7_info;
extern char* feature_menu_table_list[NUM_FEATURES];
extern char* feature_menu_items_list[NUM_FEATURES];
extern char* feature_name_list[NUM_FEATURES];

void
BuildEmptyRange(int feature)
{
  GtkWidget *table, *frame, *label1, *label2;
  char stemp[256];

  frame = gtk_frame_new (_(feature_name_list[feature-FEATURE_MIN]));
  gtk_widget_ref (frame);
  sprintf(stemp,"feature_%d_frame",feature);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (lookup_widget(commander_window,"vbox_features")), frame, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_widget_set_sensitive (frame, TRUE);

  switch (feature) {
  case FEATURE_TEMPERATURE:
    table = gtk_table_new (3, 2, FALSE);
    gtk_widget_ref (table);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label1 = gtk_label_new (_("Current:"));
    gtk_widget_ref (label1);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), "label_temp_scale_current", label1,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table), label1, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

    label2 = gtk_label_new (_("Target:"));
    gtk_widget_ref (label2);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), "label_temp_scale_target", label2,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table), label2, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

    break;
  case FEATURE_WHITE_BALANCE:
    table = gtk_table_new (3, 2, FALSE);
    gtk_widget_ref (table);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label1 = gtk_label_new (_("Blue/U-field:"));
    gtk_widget_ref (label1);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), "label_wb_scale_bu", label1,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table), label1, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

    label2 = gtk_label_new (_("Red/V-field:"));
    gtk_widget_ref (label2);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), "label_wb_scale_rv", label2,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table), label2, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

    break;

  default:
    table = gtk_table_new (1, 2, FALSE);
    gtk_widget_ref (table);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    break;
  }
}

void BuildRange(int feature)
{
  GtkAdjustment *adjustment, *adjustment2;
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  GtkWidget* scale, *scale2;

  char stemp[256];

  BuildEmptyRange(feature);

  // BUILD A NEW OPTION_MENU:

  sprintf(stemp,"feature_%d_menu",feature);
  //gtk_widget_destroy(GTK_WIDGET(lookup_widget(commander_window,stemp))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  sprintf(stemp,"feature_%d_table",feature);
  gtk_table_attach (GTK_TABLE (lookup_widget(commander_window, stemp)),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  /*
  fprintf(stderr,"feature %d: avail: %d, onoff: %d, man: %d, auto: %d, op: %d, min:%d, max:%d\n",feature-FEATURE_MIN,
	  feature_set->feature[feature-FEATURE_MIN].available,
	  feature_set->feature[feature-FEATURE_MIN].on_off_capable,
	  feature_set->feature[feature-FEATURE_MIN].manual_capable,
	  feature_set->feature[feature-FEATURE_MIN].auto_capable,
	  feature_set->feature[feature-FEATURE_MIN].one_push,
	  feature_set->feature[feature-FEATURE_MIN].min,
	  feature_set->feature[feature-FEATURE_MIN].max);
  */
  
  // BUILD MENU ITEMS ====================================================================================
  // 'off' menuitem optional addition:
  if (feature_set->feature[feature-FEATURE_MIN].on_off_capable>0) {
    glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_OFF]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			GTK_SIGNAL_FUNC (on_range_menu_activate),
			(int*)(feature*1000+RANGE_MENU_OFF)); // i is an int passed in a pointer variable. This is 'normal'.
  }
  // 'man' menuitem optional addition:
  if (feature_set->feature[feature-FEATURE_MIN].manual_capable>0) {
    glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_MAN]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			GTK_SIGNAL_FUNC (on_range_menu_activate),
			(int*)(feature*1000+RANGE_MENU_MAN));
  }
  // 'auto' menuitem optional addition:
  if (feature_set->feature[feature-FEATURE_MIN].auto_capable>0) {
    glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_AUTO]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			GTK_SIGNAL_FUNC (on_range_menu_activate),
			(int*)(feature*1000+RANGE_MENU_AUTO));
  }
  // 'single' menuitem optional addition:
  if (feature_set->feature[feature-FEATURE_MIN].one_push>0) {
    glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_SINGLE]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    gtk_signal_connect (GTK_OBJECT (glade_menuitem), "activate",
			GTK_SIGNAL_FUNC (on_range_menu_activate),
			(int*)(feature*1000+RANGE_MENU_SINGLE));
  }
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);
  
  // BUILD SCALE: ====================================================================================

  switch(feature) {
  case FEATURE_WHITE_BALANCE:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						   feature_set->feature[feature-FEATURE_MIN].min,
						   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_bu_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,stemp)), scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);

    scale2 = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale2);
    sprintf(stemp,"feature_%d_rv_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, scale2,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale2);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,stemp)), scale2, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale2, TRUE);
    gtk_scale_set_digits (GTK_SCALE (scale2), 0);

    gtk_range_set_adjustment((GtkRange*)scale,adjustment);
    gtk_range_set_adjustment((GtkRange*)scale2,adjustment2);

    // connect:
    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_WHITE_BALANCE+BU);
    gtk_signal_connect (GTK_OBJECT (adjustment2), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_WHITE_BALANCE+RV);
    break;
  case FEATURE_TEMPERATURE:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
    adjustment2=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						   feature_set->feature[feature-FEATURE_MIN].min,
						   feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_current_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,stemp)), scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);

    scale2 = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale2);
    sprintf(stemp,"feature_%d_target_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, scale2,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale2);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,stemp)), scale2, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale2, TRUE);
    gtk_scale_set_digits (GTK_SCALE (scale2), 0);

    gtk_range_set_adjustment((GtkRange*)scale,adjustment);
    gtk_range_set_adjustment((GtkRange*)scale2,adjustment2);
    // connect:
    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) FEATURE_TEMPERATURE);
    break;
  default:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].min,
						  feature_set->feature[feature-FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (commander_window), stemp, scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    sprintf(stemp,"feature_%d_table",feature);
    gtk_table_attach (GTK_TABLE (lookup_widget(commander_window,stemp)), scale, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);

    gtk_range_set_adjustment((GtkRange*)scale,adjustment);
    // connect:
    gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", GTK_SIGNAL_FUNC (on_scale_value_changed), (int*) feature);
    
  }
 
}

void
BuildFormat7Ranges(void)
{
  
  GtkAdjustment  *adjustment_px, *adjustment_py, *adjustment_sx, *adjustment_sy;
  Format7ModeInfo *info;
  
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];

  //fprintf(stderr,"size: %d %d\n",info->max_size_x,info->max_size_y);

  // define the adjustments for the 4 format7 controls. Note that (pos_x+size_x)<=max_size_x which yields some inter-dependencies

  // define adjustement for X-position
  if (info->use_unit_pos>0)
    adjustment_px=(GtkAdjustment*)gtk_adjustment_new(info->pos_x,0,info->max_size_x-info->size_x,info->step_pos_x,info->step_pos_x*4,0);
  else
    adjustment_px=(GtkAdjustment*)gtk_adjustment_new(info->pos_x,0,info->max_size_x-info->size_x,info->step_x,info->step_x*4,0);

  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_hposition_scale"),adjustment_px);
  gtk_signal_connect(GTK_OBJECT (adjustment_px), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_POS_X);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(format7_window, "format7_hposition_scale"), GTK_UPDATE_DELAYED);
  
  // define adjustement for Y-position 
  if (info->use_unit_pos>0)
    adjustment_py=(GtkAdjustment*)gtk_adjustment_new(info->pos_y,0,info->max_size_y-info->size_y,info->step_pos_y,info->step_pos_y*4,0);
  else
    adjustment_py=(GtkAdjustment*)gtk_adjustment_new(info->pos_y,0,info->max_size_y-info->size_y,info->step_y,info->step_y*4,0);

  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_vposition_scale"),adjustment_py);
  gtk_signal_connect(GTK_OBJECT (adjustment_py), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_POS_Y);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(format7_window, "format7_vposition_scale"), GTK_UPDATE_DELAYED);

  // define adjustement for X-size
  adjustment_sx=(GtkAdjustment*)gtk_adjustment_new(info->size_x,info->step_x,info->max_size_x-info->pos_x,info->step_x,info->step_x*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_hsize_scale"),adjustment_sx);
  gtk_signal_connect(GTK_OBJECT (adjustment_sx), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_SIZE_X);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(format7_window, "format7_hsize_scale"), GTK_UPDATE_DELAYED);

  // define adjustement for X-size
  adjustment_sy=(GtkAdjustment*)gtk_adjustment_new(info->size_y,info->step_y,info->max_size_y-info->pos_y,info->step_y,info->step_y*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_vsize_scale"),adjustment_sy);
  gtk_signal_connect(GTK_OBJECT (adjustment_sy), "value_changed", GTK_SIGNAL_FUNC (on_format7_value_changed), (int*) FORMAT7_SIZE_Y);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(format7_window, "format7_vsize_scale"), GTK_UPDATE_DELAYED);

  gtk_signal_emit_by_name(GTK_OBJECT (adjustment_sx), "changed");
  gtk_signal_emit_by_name(GTK_OBJECT (adjustment_sy), "changed");
  gtk_signal_emit_by_name(GTK_OBJECT (adjustment_px), "changed");
  gtk_signal_emit_by_name(GTK_OBJECT (adjustment_sy), "changed");

}

void
BuildFormat7BppRange(void)
{ 
  GtkAdjustment *adjustment_packet;
  Format7ModeInfo *info;
  info=&format7_info->mode[format7_info->edit_mode-MODE_FORMAT7_MIN];
  // define adjustment for packet size:
  adjustment_packet=(GtkAdjustment*)gtk_adjustment_new(info->bpp,info->min_bpp,info->max_bpp,1,(info->max_bpp-info->min_bpp)/16,0);
  // min_bpp is the minimum bpp, but also the 'unit' bpp.
  gtk_range_set_adjustment((GtkRange*)lookup_widget(format7_window, "format7_packet_size"),adjustment_packet);
  gtk_signal_connect(GTK_OBJECT (adjustment_packet), "value_changed", GTK_SIGNAL_FUNC (on_format7_packet_size_changed),(int*)0);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(format7_window, "format7_packet_size"), GTK_UPDATE_DELAYED);
  
}
