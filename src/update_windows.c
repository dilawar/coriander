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

#include "update_windows.h"

extern GtkWidget *commander_window;

void
UpdateFormat7Window(void)
{
  // update ranges
  UpdateFormat7Ranges();
  UpdateFormat7BppRange();
  BuildFormat7ColorMenu();

  // TODO:
  // update values in data fields
}

void
UpdatePreferencesWindow(void)
{
  UpdatePrefsReceiveFrame();    
  UpdatePrefsDisplayFrame();
  UpdatePrefsGeneralFrame();    
  UpdatePrefsSaveFrame();
  UpdatePrefsFtpFrame();
}

void
UpdateCommanderWindow(void)
{
  UpdateCameraFrame();
  UpdateTriggerFrame();
  UpdatePowerFrame();
  UpdateMemoryFrame();
  UpdateFormatMenu();
  UpdateIsoFrame();
  UpdateOptionFrame();
  UpdateServiceFrame();
}

void
UpdatePositionWindow(void)
{
  UpdateRange(commander_window, FEATURE_FOCUS);
  UpdateRange(commander_window, FEATURE_PAN);
  UpdateRange(commander_window, FEATURE_TILT);
  UpdateRange(commander_window, FEATURE_ZOOM);
}

void
UpdateColorWindow(void)
{
  UpdateRange(commander_window, FEATURE_BRIGHTNESS);
  UpdateRange(commander_window, FEATURE_GAMMA);
  UpdateRange(commander_window, FEATURE_SATURATION);
  UpdateRange(commander_window, FEATURE_HUE);
  UpdateRange(commander_window, FEATURE_WHITE_BALANCE);
  UpdateRange(commander_window, FEATURE_SHARPNESS);
}

void
UpdateApertureWindow(void)
{
  UpdateRange(commander_window, FEATURE_EXPOSURE);
  UpdateRange(commander_window, FEATURE_IRIS);
  UpdateRange(commander_window, FEATURE_SHUTTER);
  UpdateRange(commander_window, FEATURE_GAIN);
  UpdateRange(commander_window, FEATURE_OPTICAL_FILTER);
}

void
UpdateTemperatureWindow(void)
{
  UpdateRange(commander_window, FEATURE_TEMPERATURE);
}

void
UpdateStatusWindow(void)
{
  UpdateCameraStatusFrame();
  UpdateTransferStatusFrame();
}

void
UpdateCaptureWindow(void)
{
  UpdateRange(commander_window, FEATURE_CAPTURE_SIZE);
  UpdateRange(commander_window, FEATURE_CAPTURE_QUALITY);

}

void
UpdateAllWindows(void)
{
  UpdatePreferencesWindow();
  UpdatePositionWindow();
  UpdateColorWindow();
  UpdateApertureWindow();
  UpdateCaptureWindow();
  UpdateTemperatureWindow();
  UpdateStatusWindow();
  UpdateCommanderWindow();
}
