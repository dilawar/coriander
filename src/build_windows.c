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
#include "update_windows.h"
#include "build_menus.h"
#include "build_ranges.h"
#include "build_frames.h"
#include "build_windows.h"
#include "preferences.h"
#include "definitions.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *porthole_window;
extern GtkWidget *color_window;
extern GtkWidget *aperture_window;
extern GtkWidget *capture_window;
extern GtkWidget *preferences_window;
extern GtkWidget *temperature_window;
extern GtkWidget *status_window;
extern UIInfo *uiinfo;

void
BuildPreferencesWindow(void)
{
  LoadConfigFile();
  BuildPrefsRanges();
  BuildPrefsSaveFrame();
  BuildPrefsFtpFrame();
}

void
BuildFormat7Window(void)
{
  BuildFormat7ModeFrame();
}

void
BuildColorWindow(void)
{
  BuildRange(color_window, FEATURE_BRIGHTNESS);
  BuildRange(color_window, FEATURE_GAMMA);
  BuildRange(color_window, FEATURE_SATURATION);
  BuildRange(color_window, FEATURE_HUE);
  BuildRange(color_window, FEATURE_WHITE_BALANCE);
  BuildRange(color_window, FEATURE_SHARPNESS);
}

void
BuildPortholeWindow(void)
{
  BuildRange(porthole_window, FEATURE_FOCUS);
  BuildRange(porthole_window, FEATURE_PAN);
  BuildRange(porthole_window, FEATURE_TILT);
  BuildRange(porthole_window, FEATURE_ZOOM);

  uiinfo->overlay_power=0;
}

void
BuildApertureWindow(void)
{
  BuildRange(aperture_window, FEATURE_EXPOSURE);
  BuildRange(aperture_window, FEATURE_IRIS);
  BuildRange(aperture_window, FEATURE_SHUTTER);
  BuildRange(aperture_window, FEATURE_GAIN);
  BuildRange(aperture_window, FEATURE_OPTICAL_FILTER);
}

void
BuildCommanderWindow(void)
{
  BuildPowerFrame();
  BuildTriggerFrame();
  BuildIsoFrame();
  BuildCameraFrame();
  BuildMemoryFrame();
}

void
BuildCaptureWindow(void)
{
  BuildRange(capture_window, FEATURE_CAPTURE_SIZE);
  BuildRange(capture_window, FEATURE_CAPTURE_QUALITY);
}

void
BuildTemperatureWindow(void)
{
  BuildRange(temperature_window, FEATURE_TEMPERATURE);
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
  BuildPortholeWindow();
  BuildColorWindow();
  BuildApertureWindow();
  BuildCaptureWindow();
  BuildTemperatureWindow();
  BuildFormat7Window();
  BuildStatusWindow();
}
