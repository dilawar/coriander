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

extern GtkWidget *commander_window;
extern GtkWidget *help_window;
extern uiinfo_t* uiinfo;
extern int camera_num;
extern const char *help_key_bindings_keys[KEY_BINDINGS_NUM];
extern const char *help_key_bindings_functions[KEY_BINDINGS_NUM];
extern Format7Info* format7_info;
extern dc1394_miscinfo* misc_info;

void
BuildPreferencesWindow(void)
{
  LoadConfigFile();
  BuildPrefsGeneralFrame();
  BuildPrefsDisplayFrame();
  BuildPrefsReceiveFrame();
  BuildPrefsSaveFrame();
  BuildPrefsFtpFrame();
}

void
BuildFormat7Window(void)
{
  int f;
  // this window is built only if the camera supports F7. If there is a support,
  // the default edit mode is either the currently selected mode (F7 active) or
  // the first available mode (F7 inactive)

  // if we are using F7, choose current F7 mode as default
  if (misc_info->format==FORMAT_SCALABLE_IMAGE_SIZE) {
    format7_info->edit_mode=misc_info->mode;
  }
  // if we are NOT using F7, check if an F7 mode is supported and use the first one as default
  else { 
    // get first supported F7 mode
    f=MODE_FORMAT7_MIN;
    while ((format7_info->mode[f].present==0)&&(f<=MODE_FORMAT7_MAX))
      f++;
    
    if (format7_info->mode[f].present==0) {
      // F7 not supported. don't build anything
      format7_info->edit_mode=-1;
    }
    else {
      format7_info->edit_mode=f;
    }
  }

  if (format7_info->edit_mode>0) {
    BuildFormat7ModeFrame();
    BuildFormat7Ranges();
    BuildFormat7BppRange();
  }
}

void
BuildColorWindow(void)
{
  BuildRange(commander_window, FEATURE_BRIGHTNESS);
  BuildRange(commander_window, FEATURE_GAMMA);
  BuildRange(commander_window, FEATURE_SATURATION);
  BuildRange(commander_window, FEATURE_HUE);
  BuildRange(commander_window, FEATURE_WHITE_BALANCE);
  BuildRange(commander_window, FEATURE_SHARPNESS);
}

void
BuildPositionWindow(void)
{
  BuildRange(commander_window, FEATURE_FOCUS);
  BuildRange(commander_window, FEATURE_PAN);
  BuildRange(commander_window, FEATURE_TILT);
  BuildRange(commander_window, FEATURE_ZOOM);
}

void
BuildApertureWindow(void)
{
  BuildRange(commander_window, FEATURE_EXPOSURE);
  BuildRange(commander_window, FEATURE_IRIS);
  BuildRange(commander_window, FEATURE_SHUTTER);
  BuildRange(commander_window, FEATURE_GAIN);
  BuildRange(commander_window, FEATURE_OPTICAL_FILTER);
}

void
BuildCommanderWindow(void)
{
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
BuildCaptureWindow(void)
{
  BuildRange(commander_window, FEATURE_CAPTURE_SIZE);
  BuildRange(commander_window, FEATURE_CAPTURE_QUALITY);
}

void
BuildTemperatureWindow(void)
{
  BuildRange(commander_window, FEATURE_TEMPERATURE);
}

void
BuildStatusWindow(void)
{
  BuildCameraStatusFrame();
  BuildTransferStatusFrame();
}

void
BuildAllWindows(void)
{
  BuildPreferencesWindow();
  BuildCommanderWindow();
  BuildPositionWindow();
  BuildColorWindow();
  BuildApertureWindow();
  BuildCaptureWindow();
  BuildTemperatureWindow();
  BuildFormat7Window();
  BuildStatusWindow();
  BuildAbsoluteSettingsWindow();
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

void
BuildAbsoluteSettingsWindow(void)
{
  int abs_available=0;

  abs_available+=BuildAbsApertureFrame();
  abs_available+=BuildAbsColorFrame();
  abs_available+=BuildAbsLuminanceFrame();
  abs_available+=BuildAbsPositioningFrame();

  gtk_widget_set_sensitive(lookup_widget(commander_window,"absolute_settings_window"), (abs_available>0) );
}
