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
#include "update_windows.h"
#include "build_menus.h"
#include "build_ranges.h"
#include "build_frames.h"
#include "build_windows.h" 
#include "preferences.h"
#include "definitions.h"
#include <libdc1394/dc1394_control.h>

extern GtkWidget *commander_window;
extern UIInfo *uiinfo;
extern int camera_num;
void
BuildPreferencesWindow(void)
{
  LoadConfigFile();
  BuildPrefsDisplayFrame();
  BuildPrefsReceiveFrame();
  BuildPrefsSaveFrame();
  BuildPrefsFtpFrame();
  BuildPrefsRealFrame();
}

void
BuildFormat7Window(void)
{
  BuildFormat7ModeFrame();
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
  BuildCameraFrame();
  BuildMemoryFrame();
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
}
