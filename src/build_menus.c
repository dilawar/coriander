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

extern char* fps_label_list[DC1394_FRAMERATE_NUM];
extern char* format7_color_list[DC1394_MODE_FORMAT7_NUM];
extern char* format7_mode_list[DC1394_MODE_FORMAT7_NUM];
extern char* channel_num_list[16];
extern char* trigger_mode_list[4];
extern char* format0_list[DC1394_MODE_FORMAT0_NUM];
extern char* format1_list[DC1394_MODE_FORMAT1_NUM];
extern char* format2_list[DC1394_MODE_FORMAT2_NUM];
extern char* format6_list[DC1394_MODE_FORMAT6_NUM];
extern char* format7_list[DC1394_MODE_FORMAT7_NUM];

void
BuildTriggerModeMenu(void)
{
  int i, f, modes;
  quadlet_t value;
  int index[DC1394_TRIGGER_MODE_NUM];
  unsigned int current_trigger_mode;
  GtkWidget* trigger_mode;
  GtkWidget* trigger_mode_menu;
  GtkWidget* glade_menuitem;
  int k=0;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"trigger_mode"))); // remove previous menu

  trigger_mode = gtk_option_menu_new ();
  gtk_widget_ref (trigger_mode);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "trigger_mode", trigger_mode,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (trigger_mode);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table17")), trigger_mode, 0, 2, 1, 2);
  gtk_container_set_border_width (GTK_CONTAINER (trigger_mode), 1);

  trigger_mode_menu = gtk_menu_new ();

  // the following 'if' was added because the iSight from Apple does not even implement the registers over
  // offset 0x530h. Thus we can't probe anything there without producing an error
  if (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available!=0) {

  if (dc1394_query_feature_characteristics(&camera->camera_info,DC1394_FEATURE_TRIGGER,&value)!=DC1394_SUCCESS)
    MainError("Could not query trigger feature characteristics");
  modes=( (value & (0xF << 12)) >>12 );

  if (modes!=0) { // at least one mode present
    // external trigger available:
    for (f=DC1394_TRIGGER_MODE_MIN,i=0;f<=DC1394_TRIGGER_MODE_MAX;i++,f++) {
      if (modes & (0x1<<(DC1394_TRIGGER_MODE_MAX-f))) {
	index[i]=k;
	k++;
	glade_menuitem = gtk_menu_item_new_with_label (_(trigger_mode_list[i]));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (trigger_mode_menu), glade_menuitem);
	g_signal_connect ((gpointer) glade_menuitem, "activate", G_CALLBACK (on_trigger_mode_activate), (int*)f);
      }
      else
	index[i]=0;
    }

    gtk_option_menu_set_menu (GTK_OPTION_MENU (trigger_mode), trigger_mode_menu);
    
    // sets the active menu item:
    if (dc1394_get_trigger_mode(&camera->camera_info, &current_trigger_mode)!=DC1394_SUCCESS) {
      MainError("Could not query current trigger mode");
      current_trigger_mode=DC1394_TRIGGER_MODE_MIN;
    }
    //fprintf(stderr,"current trigger mode: %d\n", current_trigger_mode - TRIGGER_MODE_MIN);
    gtk_option_menu_set_history (GTK_OPTION_MENU (trigger_mode), index[current_trigger_mode - DC1394_TRIGGER_MODE_MIN]);
    
  }
  else {
    // add dummy menu item
    glade_menuitem = gtk_menu_item_new_with_label (_("N/A"));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (trigger_mode_menu), glade_menuitem);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (trigger_mode), trigger_mode_menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (trigger_mode), 0);
  }

  }
  else{
    // add dummy menu item
    glade_menuitem = gtk_menu_item_new_with_label (_("N/A"));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (trigger_mode_menu), glade_menuitem);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (trigger_mode), trigger_mode_menu);
    gtk_option_menu_set_history (GTK_OPTION_MENU (trigger_mode), 0);
  }
 
}


void
BuildMemoryChannelMenu(void)
{
  int i;
  // note: this function does not require indexing the menu items for they are always in the same consecutive order
  GtkWidget* channel_num;
  GtkWidget* channel_num_menu;
  GtkWidget* glade_menuitem;

  camera->camera_info.save_channel=camera->camera_info.load_channel;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"memory_channel"))); // remove previous menu

  channel_num = gtk_option_menu_new ();
  gtk_widget_ref (channel_num);
  gtk_object_set_data_full ((gpointer) main_window, "memory_channel", channel_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (channel_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table16")), channel_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (channel_num), 1);

  channel_num_menu = gtk_menu_new ();

  for (i=0;i<=camera->camera_info.mem_channel_number;i++) {
    glade_menuitem = gtk_menu_item_new_with_label (_(channel_num_list[i]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (channel_num_menu), glade_menuitem);
    g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_memory_channel_activate),
			(int*)i); // i is an int passed in a pointer variable. This is 'normal'.
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (channel_num), channel_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (channel_num), camera->camera_info.load_channel);
}

void
BuildCameraMenu(void)
{
  int i,current_camera_id;
  camera_t* camera_ptr;

  // note: this function does not require indexing the menu items for they are always in the same consecutive order
  GtkWidget* camera_id;
  GtkWidget* camera_id_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"camera_select"))); // remove previous menu

  //eprint("yuk\n");
  camera_id = gtk_option_menu_new ();
  gtk_widget_ref (camera_id);
  gtk_object_set_data_full ((gpointer) main_window, "camera_select", camera_id,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camera_id);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table9")), camera_id, 1, 2, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (camera_id), 1);

  camera_id_menu = gtk_menu_new (); 

  //eprint("333333\n");
  camera_ptr=cameras;
  i=0;
  current_camera_id=0;
  while (camera_ptr!=NULL) {
    glade_menuitem = gtk_menu_item_new_with_label (_(camera_ptr->prefs.name));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (camera_id_menu), glade_menuitem);
    g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_camera_select_activate),
			(camera_t*)camera_ptr);
    if (camera_ptr==camera) {
      current_camera_id=i;
    }
    camera_ptr=camera_ptr->next;
    i++;
  }
  //eprint("garb\n");

  gtk_option_menu_set_menu (GTK_OPTION_MENU (camera_id), camera_id_menu);

  //eprint("uioyiuyb\n");
  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (camera_id) , current_camera_id);

  //eprint("exit\n");
}


void
BuildFormat7ModeMenu(void)
{
  int i,f;
  int k=0;
  int index[DC1394_MODE_FORMAT7_NUM];

  //fprintf(stderr,"building F7 mode menu\n");

  GtkWidget* mode_num;
  GtkWidget* mode_num_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"format7_mode"))); // remove previous menu

  mode_num = gtk_option_menu_new ();
  gtk_widget_ref (mode_num);
  gtk_object_set_data_full ((gpointer) main_window, "format7_mode", mode_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table19")), mode_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (mode_num), 1);

  mode_num_menu = gtk_menu_new ();

  for (f=DC1394_MODE_FORMAT7_MIN,i=0;f<=DC1394_MODE_FORMAT7_MAX;f++,i++) {
    if (camera->format7_info.mode[f-DC1394_MODE_FORMAT7_MIN].present>0) {
      index[i]=k;
      k++;
      glade_menuitem = gtk_menu_item_new_with_label (_(format7_mode_list[i]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			  G_CALLBACK (on_edit_format7_mode_activate),
			  (int*)f); // i is an int passed in a pointer variable. This is 'normal'.
    }
    else
      index[i]=0;
  }
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (mode_num), mode_num_menu);

  // sets the active menu item: // default: use the first mode for edit
  //fprintf(stderr,"F7 mode: max: %d, default: %d\n",
  //	  NUM_MODE_FORMAT7,format7_info->edit_mode-MODE_FORMAT7_MIN);
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_num), 
			       index[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN]);

}


void
BuildFormat7ColorMenu(void)
{
  int i;

  //fprintf(stderr,"building F7 color menu\n");

  GtkWidget* color_num;
  GtkWidget* color_num_menu;
  GtkWidget* glade_menuitem;

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"format7_color"))); // remove previous menu

  color_num = gtk_option_menu_new ();
  gtk_widget_ref (color_num);
  gtk_object_set_data_full ((gpointer) main_window, "format7_color", color_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table19")), color_num, 2, 3, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (color_num), 1);

  color_num_menu = gtk_menu_new ();

  //eprint("ready to add\n");

  for (i=0;i<camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_codings.num;i++) {
    //eprint("%d\n",camera->format7_info.mode[camera->format7_info.edit_mode-MODE_FORMAT7_MIN].color_codings[i]);
    glade_menuitem = gtk_menu_item_new_with_label (_(format7_color_list[camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_codings.modes[i]-DC1394_COLOR_CODING_MIN]));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (color_num_menu), glade_menuitem);
    g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_edit_format7_color_activate),
		      (int*)camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_codings.modes[i]);
  }
  
  for (i=0;i<camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_codings.num;i++) {
    if (camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_coding_id==camera->format7_info.mode[camera->format7_info.edit_mode-DC1394_MODE_FORMAT7_MIN].color_codings.modes[i])
      break;
  }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (color_num), color_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (color_num), i);

}


void
BuildFpsMenu(void)
{
  int i;
  GtkWidget* fps;
  GtkWidget* fps_menu;
  GtkWidget* glade_menuitem;
  quadlet_t value;
  dc1394framerates_t framerates;
  //eprint("building framerates menu\n");

  if ((camera->camera_info.mode >= DC1394_MODE_FORMAT7_MIN) &&
      (camera->camera_info.mode <= DC1394_MODE_FORMAT7_MAX)) {
    value = 0; /* format 7 has no fixed framerates */
    gtk_widget_set_sensitive(lookup_widget(main_window,"fps_menu"),FALSE);
  }
  else {
    gtk_widget_set_sensitive(lookup_widget(main_window,"fps_menu"),TRUE);

    if (dc1394_query_supported_framerates(&camera->camera_info, camera->camera_info.mode, &framerates)!=DC1394_SUCCESS)
      MainError("Could not query supported framerates");
    gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"fps_menu"))); // remove previous menu
    
    fps = gtk_option_menu_new ();
    gtk_widget_ref (fps);
    gtk_object_set_data_full ((gpointer) main_window, "fps_menu", fps,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (fps);
    gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table17")), fps, 0, 2, 2, 3);
    gtk_container_set_border_width (GTK_CONTAINER (fps), 1);
    
    fps_menu = gtk_menu_new ();
    
    for (i=0;i<framerates.num;i++) {
	glade_menuitem = gtk_menu_item_new_with_label (_(fps_label_list[framerates.framerates[i]-DC1394_FRAMERATE_MIN]));
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (fps_menu), glade_menuitem);
	g_signal_connect ((gpointer) glade_menuitem, "activate",
			    G_CALLBACK (on_fps_activate),
			    (unsigned int*)framerates.framerates[i]);
    }
    gtk_option_menu_set_menu (GTK_OPTION_MENU (fps), fps_menu);
    
    // here we set the sensitiveness, AFTER the 'gtk_option_menu_set_menu' command:
    //gtk_widget_set_sensitive (lookup_widget(main_window,"fps_menu"),
    //			    !(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"trigger_external")))->active);
    
    // switch to nearest FPS if the previous value is not valid anymore
    for (i=0;i<framerates.num;i++) {
      if (camera->camera_info.framerate==framerates.framerates[i])
	break;
    }
    if (camera->camera_info.framerate!=framerates.framerates[i]) {
      i=SwitchToNearestFPS(&framerates, camera->camera_info.framerate);
    }
    // sets the active menu item:
    gtk_option_menu_set_history (GTK_OPTION_MENU (fps), i);
  }
  
  //eprint("finnished building framerates menu\n");
}


void
BuildFormatMenu(void)
{
  int i;
  GtkWidget* mode_num;
  GtkWidget* mode_num_menu;
  GtkWidget* glade_menuitem;
  dc1394videomodes_t modes;

  //eprint("building format menu\n");

  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window,"format_select"))); // remove previous menu

  mode_num = gtk_option_menu_new ();
  gtk_widget_ref (mode_num);
  gtk_object_set_data_full ((gpointer) main_window, "format_select", mode_num,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_num);
  gtk_table_attach_defaults (GTK_TABLE (lookup_widget(main_window,"table60")), mode_num, 0, 1, 0, 1);
  gtk_container_set_border_width (GTK_CONTAINER (mode_num), 1);

  mode_num_menu = gtk_menu_new ();

  //eprint("check\n");

  // get supported modes
  if (dc1394_query_supported_modes(&camera->camera_info, &modes)<0) {
    MainError("Could not query supported formats");
    return;
  }

  for (i=0;i<modes.num;i++) {
    if ((modes.modes[i]>=DC1394_MODE_FORMAT0_MIN)&&(modes.modes[i]<=DC1394_MODE_FORMAT0_MAX)) {
      glade_menuitem = gtk_menu_item_new_with_label (_(format0_list[modes.modes[i]-DC1394_MODE_FORMAT0_MIN]));
    }
    else if ((modes.modes[i]>=DC1394_MODE_FORMAT1_MIN)&&(modes.modes[i]<=DC1394_MODE_FORMAT1_MAX)) {
      glade_menuitem = gtk_menu_item_new_with_label (_(format1_list[modes.modes[i]-DC1394_MODE_FORMAT1_MIN]));
    }
    else if ((modes.modes[i]>=DC1394_MODE_FORMAT2_MIN)&&(modes.modes[i]<=DC1394_MODE_FORMAT2_MAX)) {
      glade_menuitem = gtk_menu_item_new_with_label (_(format2_list[modes.modes[i]-DC1394_MODE_FORMAT2_MIN]));
    }
    else if ((modes.modes[i]>=DC1394_MODE_FORMAT6_MIN)&&(modes.modes[i]<=DC1394_MODE_FORMAT6_MAX)) {
      glade_menuitem = gtk_menu_item_new_with_label (_(format6_list[modes.modes[i]-DC1394_MODE_FORMAT6_MIN]));
    }
    else if ((modes.modes[i]>=DC1394_MODE_FORMAT7_MIN)&&(modes.modes[i]<=DC1394_MODE_FORMAT7_MAX)) {
      glade_menuitem = gtk_menu_item_new_with_label (_(format7_list[modes.modes[i]-DC1394_MODE_FORMAT7_MIN]));
      //eprint("menuitem ok\n");
    }
    else {
      eprint("Invalid mode\n");
      return;
    }

    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (mode_num_menu), glade_menuitem);
    g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (ChangeModeAndFormat),
		      (int*)modes.modes[i]);
  }
  for (i=0;i<modes.num;i++) {
    if (camera->camera_info.mode==modes.modes[i])
      break;
  }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (mode_num), mode_num_menu);

  // sets the active menu item:
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_num), i);

  //eprint("finished building format menu\n");
}


void
BuildBayerMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"bayer_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "bayer_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table61")),
		    new_option_menu, 0, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // add no bayer option
  glade_menuitem = gtk_menu_item_new_with_label (_("No Bayer"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)NO_BAYER_DECODING); 
  // add nearest_neighbor option
  glade_menuitem = gtk_menu_item_new_with_label (_("Nearest"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_NEAREST); 
  // add simple option
  glade_menuitem = gtk_menu_item_new_with_label (_("Simple"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_SIMPLE); 
  // add edge sense option
  glade_menuitem = gtk_menu_item_new_with_label (_("Edge Sense"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_EDGESENSE); 
  // add downsample option
  glade_menuitem = gtk_menu_item_new_with_label (_("Downsample"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_DOWNSAMPLE); 
  // add bilinear option
  glade_menuitem = gtk_menu_item_new_with_label (_("Bilinear"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_BILINEAR); 
  // add HQ linear option
  glade_menuitem = gtk_menu_item_new_with_label (_("HQ Linear"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_menu_activate),
		      (int*)DC1394_BAYER_METHOD_HQLINEAR); 
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "bayer_menu")),camera->bayer);
  pthread_mutex_unlock(&camera->uimutex);
      
}

void
BuildBayerPatternMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"pattern_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "pattern_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table61")),
		    new_option_menu, 0, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // add BGGR option
  glade_menuitem = gtk_menu_item_new_with_label (_("BGGR"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_pattern_menu_activate),
		      (int*)DC1394_COLOR_FILTER_BGGR); 
  // add GRBG option
  glade_menuitem = gtk_menu_item_new_with_label (_("GRBG"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_pattern_menu_activate),
		      (int*)DC1394_COLOR_FILTER_GRBG); 
  // add RGGB option
  glade_menuitem = gtk_menu_item_new_with_label (_("RGGB"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_pattern_menu_activate),
		      (int*)DC1394_COLOR_FILTER_RGGB); 
  // add GBRG option
  glade_menuitem = gtk_menu_item_new_with_label (_("GBRG"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_pattern_menu_activate),
		      (int*)DC1394_COLOR_FILTER_GBRG); 
  /*
  // add Auto option
  glade_menuitem = gtk_menu_item_new_with_label (_("Auto"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_bayer_pattern_menu_activate),
		      (int*)0); 
  */
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "pattern_menu")),
			      camera->bayer_pattern);
  pthread_mutex_unlock(&camera->uimutex);
      
}


void
BuildStereoMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"stereo_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "stereo_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table61")),
		    new_option_menu, 0, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // add no stereo option
  glade_menuitem = gtk_menu_item_new_with_label (_("No Stereo"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_stereo_menu_activate),
		      (int*)NO_STEREO_DECODING); 
  // add interlaced option
  glade_menuitem = gtk_menu_item_new_with_label (_("St. Interlaced"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_stereo_menu_activate),
		      (int*)STEREO_DECODING_INTERLACED); 
  // add field option
  glade_menuitem = gtk_menu_item_new_with_label (_("St. Field"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_stereo_menu_activate),
		      (int*)STEREO_DECODING_FIELD); 
  
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "stereo_menu")),camera->stereo);
  pthread_mutex_unlock(&camera->uimutex);
      
}

void
BuildOverlayPatternMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"overlay_pattern_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "overlay_pattern_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table80")),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // OFF
  glade_menuitem = gtk_menu_item_new_with_label (_("OFF"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_OFF);
 
  // Rectangle
  glade_menuitem = gtk_menu_item_new_with_label (_("Rectangle"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_RECTANGLE);

  // Small cross
  glade_menuitem = gtk_menu_item_new_with_label (_("Small Cross"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_SMALL_CROSS);
  
  // Large cross
  glade_menuitem = gtk_menu_item_new_with_label (_("Large Cross"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_LARGE_CROSS);
  
  // Golden mean (rem: golden mean is more complex and involves the golden ratio of 1.618...
  // The "thirds" is a simplification
  glade_menuitem = gtk_menu_item_new_with_label (_("Thirds"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_GOLDEN_MEAN);

  // over and under exposed regions
  glade_menuitem = gtk_menu_item_new_with_label (_("Exposure check"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_OVER_UNDEREXPOSED);
 
  /*
  // Image
  glade_menuitem = gtk_menu_item_new_with_label (_("Custom image"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_pattern_menu_activate),
		      (int*)OVERLAY_PATTERN_IMAGE);
  */
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "overlay_pattern_menu")),camera->prefs.overlay_pattern);
  pthread_mutex_unlock(&camera->uimutex);
      
}

void
BuildOverlayTypeMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"overlay_type_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "overlay_type_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table80")),
		    new_option_menu, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // Replace
  glade_menuitem = gtk_menu_item_new_with_label (_("Replace"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_type_menu_activate),
		      (int*)OVERLAY_TYPE_REPLACE);

  // Random
  glade_menuitem = gtk_menu_item_new_with_label (_("Random"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_type_menu_activate),
		      (int*)OVERLAY_TYPE_RANDOM);

  // Invert
  glade_menuitem = gtk_menu_item_new_with_label (_("Invert"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_type_menu_activate),
		      (int*)OVERLAY_TYPE_INVERT);
  
  /*  
  // Average
  glade_menuitem = gtk_menu_item_new_with_label (_("Average"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_overlay_type_menu_activate),
		      (int*)OVERLAY_TYPE_AVERAGE);
  */
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "overlay_type_menu")),camera->prefs.overlay_type);
  pthread_mutex_unlock(&camera->uimutex);
  
}

void
BuildSaveFormatMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"save_format_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "save_format_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table82")),
		    new_option_menu, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // DUMMY
  glade_menuitem = gtk_menu_item_new_with_label (_("--- Still: ---"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (UpdateSaveFilenameFrame),
		      (void*)0);
  
  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("raw (still)"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_RAW);
  
  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("ppm/pgm"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_PPMPGM);
  
#ifdef HAVE_FFMPEG
  // only available with ffmpeg since we don;t have imlib anymore
  glade_menuitem = gtk_menu_item_new_with_label (_("jpeg"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_JPEG);
#endif  

  // DUMMY
  glade_menuitem = gtk_menu_item_new_with_label (_("--- Video: ---"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (UpdateSaveFilenameFrame),
		      (void*)0);
  
  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("raw (video)"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_RAW_VIDEO);

  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("pvn"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_PVN);
#ifdef HAVE_FFMPEG
  //
  glade_menuitem = gtk_menu_item_new_with_label (_("mpeg"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_format_menu_activate),
		      (int*)SAVE_FORMAT_MPEG);
#endif

  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  UpdateSaveFilenameFrame();
      
}

void
BuildSaveAppendMenu(void)
{
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;

  // build bayer option menu:
  gtk_widget_destroy(GTK_WIDGET(lookup_widget(main_window,"save_append_menu"))); // remove previous menu
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full ((gpointer) main_window, "save_append_menu", new_option_menu,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (GTK_TABLE (lookup_widget(main_window,"table82")),
		    new_option_menu, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("number"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_append_menu_activate),
		      (int*)SAVE_APPEND_NUMBER);
  
  // 
  glade_menuitem = gtk_menu_item_new_with_label (_("date and time"));
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  g_signal_connect ((gpointer) glade_menuitem, "activate",
		      G_CALLBACK (on_save_append_menu_activate),
		      (int*)SAVE_APPEND_DATE_TIME);
  
  if ((camera->prefs.save_to_dir==0)||(camera->prefs.save_format>=SAVE_FORMAT_RAW_VIDEO)) {
    // 
    glade_menuitem = gtk_menu_item_new_with_label (_("none (overwrite)"));
    gtk_widget_show (glade_menuitem);
    gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
    g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_save_append_menu_activate),
			(int*)SAVE_APPEND_NONE);
  }
  else {
    // if the mode is scratch we switch to something else because scratch is not allowed in save-to-dir mode
    if (camera->prefs.save_append==SAVE_APPEND_NONE) {
      camera->prefs.save_append=SAVE_APPEND_NUMBER;
      gnome_config_set_int("coriander/save/append",camera->prefs.save_append);
      gnome_config_sync();
    }
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // menu history
  pthread_mutex_lock(&camera->uimutex);
  gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "save_append_menu")),camera->prefs.save_append);
  pthread_mutex_unlock(&camera->uimutex);
      
}
