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

// these defs are a little changed compared to the dc1394 files
// but it is necessary for the use of this interface...

// the following lists are all the 'standard' controls available through the interface

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "definitions.h"
#include "preferences.h"
#include <libdc1394/dc1394_control.h>

const char * feature_list[NUM_FEATURES] = { 
  "brightness",
  "auto_exposure",
  "sharpness",
  "white_balance",
  "hue",
  "saturation",
  "gamma",
  "shutter",
  "gain",
  "iris",
  "focus",
  "temperature",
  "trigger",
  "zoom",
  "pan",
  "tilt",
  "filter",
  "capture_size",
  "capture_quality"
};

const char * feature_menu_list[NUM_FEATURES] = {
  "brightness_menu",
  "auto_exposure_menu",
  "sharpness_menu",
  "white_balance_menu",
  "hue_menu",
  "saturation_menu",
  "gamma_menu",
  "shutter_menu",
  "gain_menu",
  "iris_menu",
  "focus_menu",
  "temperature_menu",
  "trigger_menu",
  "zoom_menu",
  "pan_menu",
  "tilt_menu",
  "filter_menu",
  "capture_size_menu",
  "capture_quality_menu"
};


const char * feature_scale_list[NUM_FEATURES] = {
  "brightness_scale",
  "auto_exposure_scale",
  "sharpness_scale",
  "white_balance_scale",
  "hue_scale",
  "saturation_scale",
  "gamma_scale",
  "shutter_scale",
  "gain_scale",
  "iris_scale",
  "focus_scale",
  "temperature_scale",
  "trigger_count",
  "zoom_scale",
  "pan_scale",
  "tilt_scale",
  "filter_scale",
  "capture_size_scale",
  "capture_quality_scale"
};

const char * feature_frame_list[NUM_FEATURES] = {
  "brightness_frame",
  "auto_exposure_frame",
  "sharpness_frame",
  "white_balance_frame",
  "hue_frame",
  "saturation_frame",
  "gamma_frame",
  "shutter_frame",
  "gain_frame",
  "iris_frame",
  "focus_frame",
  "temperature_frame",
  "trigger_frame",
  "zoom_frame",
  "pan_frame",
  "tilt_frame",
  "filter_frame",
  "capture_size_frame",
  "capture_quality_frame"
};

const char * feature_menu_table_list[NUM_FEATURES] = {
  "table28",
  "table33",
  "table30",
  "table38",
  "table26",
  "table27",
  "table29",
  "table35",
  "table36",
  "table34",
  "table41",
  "table39",
  "table17",
  "table40",
  "table43",
  "table42",
  "table37",
  "table32",
  "table31"
};

const char * trigger_mode_list[NUM_TRIGGER_MODE] = {
  "Mode 0",
  "Mode 1",
  "Mode 2",
  "Mode 3"
};

const char * channel_num_list[16] = {
  "Factory",
  "Setup 1",
  "Setup 2",
  "Setup 3",
  "Setup 4",
  "Setup 5",
  "Setup 6",
  "Setup 7",
  "Setup 8",
  "Setup 9",
  "Setup 10",
  "Setup 11",
  "Setup 12",
  "Setup 13",
  "Setup 14",
  "Setup 15"
};

const char * fps_label_list[NUM_FRAMERATES] = {
  "1.875 fps",
  "3.75 fps",
  "7.5 fps",
  "15 fps",
  "30 fps",
  "60 fps"
};

const char * format7_mode_list[NUM_MODE_FORMAT7] = {
  "Mode 0",
  "Mode 1",
  "Mode 2",
  "Mode 3",
  "Mode 4",
  "Mode 5",
  "Mode 6",
  "Mode 7"
};

const char * format7_color_list[NUM_COLOR_FORMAT7] = {
  "Mono 8bpp",
  "YUV 4:1:1",
  "YUV 4:2:2",
  "YUV 4:4:4",
  "RGB 24bpp",
  "Mono 16bpp",
  "RGB 48bpp"
};

const char * phy_speed_list[4] = {
  "100 Mbps",
  "200 Mbps",
  "400 Mbps",
  "Unknown"
};

const char * power_class_list[8] = {
  "None",
  "+15W",
  "+30W",
  "+45W",
  "-1W",
  "-3W",
  "-6W",
  "-10W"
};

const char * phy_delay_list[4] = {
  "<=144ns",
  "Unknown",
  "Unknown",
  "Unknown"
};


const char * format_list[5]= {
  "format_0",
  "format_1",
  "format_2",
  "format_6",
  "format_7"
};

const char * format0_list[7]= {
  "f0_mode0",
  "f0_mode1",
  "f0_mode2",
  "f0_mode3",
  "f0_mode4",
  "f0_mode5",
  "f0_mode6"
};

const char * format1_list[8]= {
  "f1_mode0",
  "f1_mode1",
  "f1_mode2",
  "f1_mode3",
  "f1_mode4",
  "f1_mode5",
  "f1_mode6",
  "f1_mode7"
};

const char * format2_list[8]= {
  "f2_mode0",
  "f2_mode1",
  "f2_mode2",
  "f2_mode3",
  "f2_mode4",
  "f2_mode5",
  "f2_mode6",
  "f2_mode7"
};

const char * format6_list[1]= {
  "f6_mode0"
};

const char * format7_list[8]= {
  "f7_mode0",
  "f7_mode1",
  "f7_mode2",
  "f7_mode3",
  "f7_mode4",
  "f7_mode5",
  "f7_mode6",
  "f7_mode7"
};

const char *preferences_features[PREFERENCE_ITEMS]= {
  "ONE_PUSH_TIMEOUT",
  "AUTO_UPDATE",
  "AUTO_UPDATE_FREQUENCY",
  "DISPLAY_METHOD",
  "DISPLAY_PERIOD",
  "RECEIVE_METHOD",
  "SAVE_FILENAME",
  "SAVE_SCRATCH",
  "SAVE_PERIOD",
  "FTP_ADDRESS",
  "FTP_USER",
  "FTP_PASSWORD",
  "FTP_FILENAME",
  "FTP_PATH",
  "FTP_SCRATCH",
  "FTP_PERIOD",
  "REAL_ADDRESS",
  "REAL_USER",
  "REAL_PASSWORD",
  "REAL_FILENAME",
  "REAL_PORT",
  "REAL_TITLE",
  "REAL_AUTHOR",
  "REAL_COPYRIGHT",
  "REAL_RECORDABLE",
  "REAL_AUDIENCE",
  "REAL_QUALITY",
  "REAL_COMPATIBILITY",
  "REAL_PERIOD"
};

const char *feature_menu_items_list[5]= {
  "OFF",
  "Man",
  "Auto",
  "Single",
  "N/A"
};
