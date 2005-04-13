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
ErrorPopup(char * string)
{
  GtkWidget *error_box;
  GtkWidget *dialog_vbox;
  GtkWidget *dialog_action_area;
  GtkWidget *error_popup_button;
  char tmp_str[STRING_SIZE];

  sprintf(tmp_str,"ERROR: %s",string);

  error_box = gnome_message_box_new (_(tmp_str),
                              GNOME_MESSAGE_BOX_ERROR, NULL);
  gtk_widget_set_name (error_box, "error_box");
  gtk_window_set_title (GTK_WINDOW (error_box), _("Coriander error"));
  gtk_window_set_resizable (GTK_WINDOW (error_box), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (error_box), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox = GNOME_DIALOG (error_box)->vbox;
  gtk_widget_set_name (dialog_vbox, "dialog_vbox");
  gtk_widget_show (dialog_vbox);

  dialog_action_area = GNOME_DIALOG (error_box)->action_area;
  gtk_widget_set_name (dialog_action_area, "dialog_action_area");
  gtk_widget_show (dialog_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

  gnome_dialog_append_button (GNOME_DIALOG (error_box), "gtk-ok");
  error_popup_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (error_box)->buttons)->data);
  gtk_widget_set_name (error_popup_button, "error_popup_button");
  gtk_widget_show (error_popup_button);
  GTK_WIDGET_SET_FLAGS (error_popup_button, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) error_popup_button, "clicked",
                    G_CALLBACK (on_error_popup_button_activate), NULL);

  gtk_widget_set_sensitive(main_window,0);

  gtk_widget_show(error_box);

}


void MainError(const char *string)
{
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  sprintf(temp,"ERROR: %s\n",string);
  if (main_window !=NULL) {
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(lookup_widget(main_window,"main_status"))),string, -1);
  }
  free(temp);
}

void MainStatus(const char *string)
{
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(temp,"%s\n",string);
  if (main_window !=NULL) {
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(lookup_widget(main_window,"main_status"))),string, -1);
  }
  free(temp);
}

static void MessageBox_clicked (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy( GTK_WIDGET(data));
}

static void MessageBox_destroy (GtkWidget *widget, gpointer data)
{
    gtk_grab_remove (GTK_WIDGET(widget));
}

void MessageBox( gchar *message)
{
  static GtkWidget *label;
  GtkWidget *button;
  GtkWidget *dialog_window;
  
  dialog_window = gtk_dialog_new();
  g_signal_connect( (gpointer)dialog_window, "destroy", G_CALLBACK (MessageBox_destroy), dialog_window);
  gtk_window_set_title (GTK_WINDOW(dialog_window), "Coriander Message");
  gtk_container_border_width (GTK_CONTAINER(dialog_window), 5);
  label = gtk_label_new (message);
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog_window)->vbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  button = gtk_button_new_with_label ("OK");
  g_signal_connect ((gpointer) button, "clicked", G_CALLBACK (MessageBox_clicked), dialog_window);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog_window)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  gtk_widget_show (dialog_window);
  gtk_grab_add (dialog_window);
}
