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
  GtkWidget* handlebox1;
  GtkWidget* notebook2;
  GtkWidget* label59;
  GtkWidget* vbox34;
  GtkWidget* format7_mode_frame;
  GtkWidget* table19;
  GtkWidget* label13;
  GtkWidget* format7_packet_frame;
  GtkWidget* hbox57;
  GtkWidget* format7_packet_size;
  GtkWidget* format7_horizontal_frame;
  GtkWidget* table70;
  GtkWidget* label139;
  GtkWidget* label140;
  GtkWidget* format7_vertical_frame;
  GtkWidget* table71;
  GtkWidget* label141;
  GtkWidget* label142;
  GtkWidget* format7_frame_info_frame;
  GtkWidget* table15;
  GtkWidget* label14;
  GtkWidget* label15;
  GtkWidget* format7_imagebytes;
  GtkWidget* format7_imagepixels;
  GtkWidget* label159;
  GtkWidget* label160;
  GtkWidget* format7_padding;
  GtkWidget* format7_totalbytes;
  GtkWidget* format7_hsize_scale;
  GtkWidget* format7_vsize_scale;
  GtkWidget* format7_hposition_scale;
  GtkWidget* format7_vposition_scale;

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
    //fprintf(stderr,"page removed\n");
  }

  if (camera->format7_info.edit_mode>=0) {
    // we should build the full tab here

    // build a new box for F7
    handlebox1 = gtk_handle_box_new ();
    gtk_widget_ref (handlebox1);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "handlebox1", handlebox1,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (handlebox1);
    //fprintf(stderr,"handlebox set\n");

    label59 = gtk_label_new (_("Format 7"));
    gtk_widget_ref (label59);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_format7_page", label59,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label59);
    //fprintf(stderr,"label set\n");

    // the format7 tab should be placed in position 3 here:
    gtk_notebook_insert_page(GTK_NOTEBOOK (notebook2), GTK_WIDGET(handlebox1),label59, 3);
    //fprintf(stderr,"new page added\n");

    // big vbox for the whole tab
    vbox34 = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (vbox34);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox34", vbox34,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vbox34);
    gtk_container_add (GTK_CONTAINER (handlebox1), vbox34);

    // mode frame
    format7_mode_frame = gtk_frame_new (_("Current mode"));
    gtk_widget_ref (format7_mode_frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_mode_frame", format7_mode_frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_mode_frame);
    gtk_box_pack_start (GTK_BOX (vbox34), format7_mode_frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (format7_mode_frame), 5);

    table19 = gtk_table_new (1, 3, FALSE);
    gtk_widget_ref (table19);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table19", table19,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table19);
    gtk_container_add (GTK_CONTAINER (format7_mode_frame), table19);

    label13 = gtk_label_new (_("Coding:"));
    gtk_widget_ref (label13);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label13", label13,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label13);
    gtk_table_attach (GTK_TABLE (table19), label13, 1, 2, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_padding (GTK_MISC (label13), 6, 0);

    // bytes_per_packet tab
    format7_packet_frame = gtk_frame_new (_("Packet size"));
    gtk_widget_ref (format7_packet_frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_packet_frame", format7_packet_frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_packet_frame);
    gtk_box_pack_start (GTK_BOX (vbox34), format7_packet_frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (format7_packet_frame), 5);

    hbox57 = gtk_hbox_new (FALSE, 0);
    gtk_widget_ref (hbox57);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "hbox57", hbox57,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (hbox57);
    gtk_container_add (GTK_CONTAINER (format7_packet_frame), hbox57);

    format7_packet_size = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
    gtk_widget_ref (format7_packet_size);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_packet_size", format7_packet_size,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_packet_size);
    gtk_box_pack_start (GTK_BOX (hbox57), format7_packet_size, TRUE, TRUE, 0);
    gtk_scale_set_digits (GTK_SCALE (format7_packet_size), 0);

    // horizontal setup
    format7_horizontal_frame = gtk_frame_new (_("Horizontal setup"));
    gtk_widget_ref (format7_horizontal_frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_horizontal_frame", format7_horizontal_frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_horizontal_frame);
    gtk_box_pack_start (GTK_BOX (vbox34), format7_horizontal_frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (format7_horizontal_frame), 5);
    
    table70 = gtk_table_new (2, 2, FALSE);
    gtk_widget_ref (table70);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table70", table70,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table70);
    gtk_container_add (GTK_CONTAINER (format7_horizontal_frame), table70);

    label139 = gtk_label_new (_("Size"));
    gtk_widget_ref (label139);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label139", label139,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label139);
    gtk_table_attach (GTK_TABLE (table70), label139, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label139), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label139), 2, 2);

    label140 = gtk_label_new (_("Position"));
    gtk_widget_ref (label140);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label140", label140,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label140);
    gtk_table_attach (GTK_TABLE (table70), label140, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label140), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label140), 2, 2);

    format7_hsize_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (format7_hsize_scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hsize_scale", format7_hsize_scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_hsize_scale);
    gtk_table_attach (GTK_TABLE (table70), format7_hsize_scale, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (format7_hsize_scale), 0);
    
    format7_hposition_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (format7_hposition_scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hposition_scale", format7_hposition_scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_hposition_scale);
    gtk_table_attach (GTK_TABLE (table70), format7_hposition_scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (format7_hposition_scale), 0);

    // vertical setup
    format7_vertical_frame = gtk_frame_new (_("Vertical setup"));
    gtk_widget_ref (format7_vertical_frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vertical_frame", format7_vertical_frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_vertical_frame);
    gtk_box_pack_start (GTK_BOX (vbox34), format7_vertical_frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (format7_vertical_frame), 5);

    table71 = gtk_table_new (2, 2, FALSE);
    gtk_widget_ref (table71);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table71", table71,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table71);
    gtk_container_add (GTK_CONTAINER (format7_vertical_frame), table71);

    label141 = gtk_label_new (_("Size"));
    gtk_widget_ref (label141);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label141", label141,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label141);
    gtk_table_attach (GTK_TABLE (table71), label141, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label141), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label141), 2, 2);
    
    label142 = gtk_label_new (_("Position"));
    gtk_widget_ref (label142);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label142", label142,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label142);
    gtk_table_attach (GTK_TABLE (table71), label142, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label142), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label142), 2, 2);

    format7_vsize_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (format7_vsize_scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vsize_scale", format7_vsize_scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_vsize_scale);
    gtk_table_attach (GTK_TABLE (table71), format7_vsize_scale, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (format7_vsize_scale), 0);
    
    format7_vposition_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (format7_vposition_scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vposition_scale", format7_vposition_scale,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_vposition_scale);
    gtk_table_attach (GTK_TABLE (table71), format7_vposition_scale, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_digits (GTK_SCALE (format7_vposition_scale), 0);


    // information frame
    format7_frame_info_frame = gtk_frame_new (_("Frame info [bytes]"));
    gtk_widget_ref (format7_frame_info_frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_frame_info_frame", format7_frame_info_frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_frame_info_frame);
    gtk_box_pack_start (GTK_BOX (vbox34), format7_frame_info_frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (format7_frame_info_frame), 5);
    
    table15 = gtk_table_new (2, 4, TRUE);
    gtk_widget_ref (table15);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table15", table15,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table15);
    gtk_container_add (GTK_CONTAINER (format7_frame_info_frame), table15);
    
    label14 = gtk_label_new (_("Image pixels :  "));
    gtk_widget_ref (label14);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label14", label14,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label14);
    gtk_table_attach (GTK_TABLE (table15), label14, 0, 1, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label14), 2, 2);
    
    label15 = gtk_label_new (_("Image size :  "));
    gtk_widget_ref (label15);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label15", label15,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label15);
    gtk_table_attach (GTK_TABLE (table15), label15, 0, 1, 1, 2,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label15), 2, 2);
    
    format7_imagebytes = gtk_statusbar_new ();
    gtk_widget_ref (format7_imagebytes);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagebytes", format7_imagebytes,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_imagebytes);
    gtk_table_attach (GTK_TABLE (table15), format7_imagebytes, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 2, 2);
    
    format7_imagepixels = gtk_statusbar_new ();
    gtk_widget_ref (format7_imagepixels);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagepixels", format7_imagepixels,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_imagepixels);
    gtk_table_attach (GTK_TABLE (table15), format7_imagepixels, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

    label159 = gtk_label_new (_("Padding :  "));
    gtk_widget_ref (label159);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label159", label159,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label159);
    gtk_table_attach (GTK_TABLE (table15), label159, 2, 3, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label159), 2, 2);
    
    label160 = gtk_label_new (_("Total size:  "));
    gtk_widget_ref (label160);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label160", label160,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label160);
    gtk_table_attach (GTK_TABLE (table15), label160, 2, 3, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label160), 2, 2);
    
    format7_padding = gtk_statusbar_new ();
    gtk_widget_ref (format7_padding);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_padding", format7_padding,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_padding);
    gtk_table_attach (GTK_TABLE (table15), format7_padding, 3, 4, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    
    format7_totalbytes = gtk_statusbar_new ();
    gtk_widget_ref (format7_totalbytes);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_totalbytes", format7_totalbytes,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (format7_totalbytes);
    gtk_table_attach (GTK_TABLE (table15), format7_totalbytes, 3, 4, 1, 2,
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
