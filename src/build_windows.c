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

#include "build_windows.h" 

extern GtkWidget *main_window;
extern GtkWidget *help_window;
extern const char *help_key_bindings_keys[KEY_BINDINGS_NUM];
extern const char *help_key_bindings_functions[KEY_BINDINGS_NUM];
extern camera_t* camera;
extern unsigned int format7_tab_presence;

void
BuildPreferencesWindow(void)
{
  LoadConfigFile();
  BuildPrefsGeneralFrame();
  BuildPrefsDisplayFrame();
  BuildPrefsReceiveFrame();
  BuildPrefsSaveFrame();
  BuildPrefsFtpFrame();
  BuildPrefsV4lFrame();
}

void
BuildFormat7Window(void)
{
  int f;
  GtkWidget* notebook2;
  GtkWidget* vbox;
  GtkWidget* hbox;
  GtkWidget* table;
  GtkWidget* frame;
  GtkWidget* label;
  GtkWidget* bar;
  GtkWidget* scale;

  // this window is built only if the camera supports F7. If there is a support,
  // the default edit mode is either the currently selected mode (F7 active) or
  // the first available mode (F7 inactive)

  // if we are using F7, choose current F7 mode as default
  if (camera->misc_info.format==FORMAT_SCALABLE_IMAGE_SIZE) {
    camera->format7_info.edit_mode=camera->misc_info.mode;
  }
  // if we are NOT using F7, check if an F7 mode is supported and use the first one as default
  else { 
    // get first supported F7 mode
    for (f=MODE_FORMAT7_MIN;f<=MODE_FORMAT7_MAX;f++) {
      if (camera->format7_info.mode[f-MODE_FORMAT7_MIN].present>0) {
	f++;
	break;
      }
    }
    f--;

    if (camera->format7_info.mode[f-MODE_FORMAT7_MIN].present==0) {
      // F7 not supported. don't build anything
      camera->format7_info.edit_mode=-1;
    }
    else {
      camera->format7_info.edit_mode=f;
    }
  }

  notebook2=lookup_widget(main_window,"notebook2");
  // if the page exists, remove it:
  if (format7_tab_presence==1) {
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2),3);
    format7_tab_presence=0;
  }

  if (camera->format7_info.edit_mode>=0) {

    label = gtk_label_new (_("Format 7"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_format7_page", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);

    // big vbox for the whole tab
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (vbox);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox34", vbox,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vbox);

    // the format7 tab should be placed in position 3 here:
    gtk_notebook_insert_page(GTK_NOTEBOOK (notebook2), GTK_WIDGET(vbox),label, 3);

    // mode frame
    frame = gtk_frame_new (_("Current mode"));
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_mode_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    table = gtk_table_new (1, 3, FALSE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table19", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label = gtk_label_new (_("Coding:"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label13", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_padding (GTK_MISC (label), 6, 0);

    // bytes_per_packet tab
    frame = gtk_frame_new (_("Packet size"));
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_packet_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (hbox);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "hbox57", hbox,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (frame), hbox);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_packet_size", scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);

    // horizontal setup
    frame = gtk_frame_new (_("Horizontal setup"));
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_horizontal_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    
    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table70", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label = gtk_label_new (_("Size"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label139", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    label = gtk_label_new (_("Position"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label140", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hsize_scale", scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);
    
    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hposition_scale", scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);

    // vertical setup
    frame = gtk_frame_new (_("Vertical setup"));
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vertical_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table71", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label = gtk_label_new (_("Size"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label141", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    label = gtk_label_new (_("Position"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label142", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vsize_scale", scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);
    
    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vposition_scale", scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (scale), 0);


    // information frame
    frame = gtk_frame_new (_("Frame info [bytes]"));
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_frame_info_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    
    table = gtk_table_new (2, 4, TRUE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table15", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    
    label = gtk_label_new (_("Image pixels :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label14", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    label = gtk_label_new (_("Image size :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label15", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagebytes", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagepixels", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

    label = gtk_label_new (_("Padding :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label159", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    label = gtk_label_new (_("Total size:  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label160", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_padding", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 3, 4, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_totalbytes", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 3, 4, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    

    // now build the ranges and menus for this frame:
    BuildFormat7ModeFrame();
    BuildFormat7Ranges();
    BuildFormat7BppRange();
    format7_tab_presence=1;
  }

}

void
BuildFeatureWindow(void)
{
  GtkWidget* vbox_features;
  int i;
  // destroy previous feature vbox
  gtk_widget_destroy(lookup_widget(main_window,"vbox_features"));

  // build new feature vbox
  vbox_features = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox_features);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox_features", vbox_features,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox_features);
  gtk_container_add (GTK_CONTAINER (lookup_widget(main_window,"viewport1")), vbox_features);

  for (i=FEATURE_MIN;i<=FEATURE_MAX;i++) {
    if ((camera->feature_set.feature[i-FEATURE_MIN].available>0)&&
	(i!=FEATURE_TRIGGER)) {
      BuildRange(i);
    }
  }
}

void
BuildMainWindow(void)
{
  gtk_menu_item_right_justify(GTK_MENU_ITEM (lookup_widget(main_window,"help")));
  BuildPowerFrame();
  BuildServiceFrame();
  BuildTriggerFrame();
  BuildIsoFrame();
  BuildGlobalIsoFrame();
  BuildCameraFrame();
  BuildMemoryFrame();
  BuildFormatMenu();
  BuildOptionFrame();
}

void
BuildStatusWindow(void)
{
  BuildCameraStatusFrame();
  BuildTransferStatusFrame();
  BuildBandwidthFrame();
}

void
BuildAllWindows(void)
{
  BuildPreferencesWindow();
  BuildMainWindow();
  BuildFeatureWindow();
  BuildFormat7Window();
  BuildStatusWindow();
}

void
BuildHelpWindow(void)
{
  int i;
  GtkCList* clist;
  char *text[2];
  clist=GTK_CLIST(lookup_widget(help_window,"key_bindings"));

  text[0]=(char*)malloc(STRING_SIZE*sizeof(char));
  text[1]=(char*)malloc(STRING_SIZE*sizeof(char));

  gtk_clist_set_column_justification(clist,0,GTK_JUSTIFY_CENTER);
  for (i=0;i<KEY_BINDINGS_NUM;i++) {
    strcpy(text[0],help_key_bindings_keys[i]);
    strcpy(text[1],help_key_bindings_functions[i]);
    gtk_clist_append(clist,text);
  }
  gtk_clist_set_column_auto_resize(clist,0,1);
  gtk_clist_set_column_auto_resize(clist,1,1);

  free(text[0]);
  free(text[1]);

}
