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
#include <libdc1394/dc1394_control.h>

const char * feature_list[NUM_FEATURES] = {
    "brightness",
    "exposure",
    "sharpness",
    "whitebal",
    "hue",
    "saturation",
    "gamma",
    "shutter",
    "gain",
    "iris",
    "focus",
    "temp",
    "trigger",
    "zoom",
    "pan",
    "tilt",
    "filter",
    "capsize",
    "capqual"
};

const char * feature_op_list[NUM_FEATURES] = {
    "brightness_op",
    "exposure_op",
    "sharpness_op",
    "whitebal_op",
    "hue_op",
    "saturation_op",
    "gamma_op",
    "shutter_op",
    "gain_op",
    "iris_op",
    "focus_op",
    "temp_op",
    "trigger_op",
    "zoom_op",
    "pan_op",
    "tilt_op",
    "filter_op",
    "capsize_op",
    "capqual_op"
};

const char * feature_man_list[NUM_FEATURES] = {
    "brightness_man",
    "exposure_man",
    "sharpness_man",
    "whitebal_man",
    "hue_man",
    "saturation_man",
    "gamma_man",
    "shutter_man",
    "gain_man",
    "iris_man",
    "focus_man",
    "temp_man",
    "trigger_man",
    "zoom_man",
    "pan_man",
    "tilt_man",
    "filter_man",
    "capsize_man",
    "capqual_man"
};

const char * feature_auto_list[NUM_FEATURES] = {
    "brightness_auto",
    "exposure_auto",
    "sharpness_auto",
    "whitebal_auto",
    "hue_auto",
    "saturation_auto",
    "gamma_auto",
    "shutter_auto",
    "gain_auto",
    "iris_auto",
    "focus_auto",
    "temp_auto",
    "trigger_auto",
    "zoom_auto",
    "pan_auto",
    "tilt_auto",
    "filter_auto",
    "capsize_auto",
    "capqual_auto"
};

const char * feature_power_list[NUM_FEATURES] = {
    "brightness_power",
    "exposure_power",
    "sharpness_power",
    "whitebal_power",
    "hue_power",
    "saturation_power",
    "gamma_power",
    "shutter_power",
    "gain_power",
    "iris_power",
    "focus_power",
    "temp_power",
    "trigger_power",
    "zoom_power",
    "pan_power",
    "tilt_power",
    "filter_power",
    "capsize_power",
    "capqual_power"
};

const char * feature_scale_list[NUM_FEATURES] = {
    "brightness_scale",
    "exposure_scale",
    "sharpness_scale",
    "whitebal_scale",
    "hue_scale",
    "saturation_scale",
    "gamma_scale",
    "shutter_scale",
    "gain_scale",
    "iris_scale",
    "focus_scale",
    "temp_scale",
    "trigger_count",
    "zoom_scale",
    "pan_scale",
    "tilt_scale",
    "filter_scale",
    "capsize_scale",
    "capqual_scale"
};

const char * feature_frame_list[NUM_FEATURES] = {
    "brightness_frame",
    "exposure_frame",
    "sharpness_frame",
    "whitebal_frame",
    "hue_frame",
    "saturation_frame",
    "gamma_frame",
    "shutter_frame",
    "gain_frame",
    "iris_frame",
    "focus_frame",
    "temp_frame",
    "trigger_frame",
    "zoom_frame",
    "pan_frame",
    "tilt_frame",
    "filter_frame",
    "capsize_frame",
    "capqual_frame"
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

