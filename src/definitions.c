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

#include "definitions.h"

const char * feature_name_list[NUM_FEATURES] = {
  "Brightness",
  "Auto exposure",
  "Sharpness",
  "White Balance",
  "Hue",
  "Saturation",
  "Gamma",
  "Shutter",
  "Gain",
  "Iris",
  "Focus",
  "Temperature",
  "Trigger",
  "Zoom",
  "Pan",
  "Tilt",
  "Filter",
  "Capture Size",
  "Capture Quality"
};

const char *feature_abs_switch_list[NUM_FEATURES] = {
  "abs_brightness_switch",
  "abs_autoexp_switch",
  "abs_sharpness_switch",
  "abs_whitebal_switch",
  "abs_hue_switch",
  "abs_saturation_switch",
  "abs_gamma_switch",
  "abs_shutter_switch",
  "abs_gain_switch",
  "abs_iris_switch",
  "abs_focus_switch",
  "abs_temperature_switch",
  "abs_trigger_switch",
  "abs_zoom_switch",
  "abs_pan_switch",
  "abs_tilt_switch",
  "abs_filter_switch",
  "abs_capture_size_switch",
  "abs_capture_quality_switch"
};

const char *feature_abs_label_list[NUM_FEATURES] = {
  "%",
  "IL",
  "-",
  "K",
  "deg",
  "%",
  "-",
  "s",
  "dB",
  "F/",
  "m",
  "-",
  "-",
  "X",
  "deg",
  "deg",
  "-",
  "-",
  "-"
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
  "unknown"
};

const char * power_class_list[8] = {
  "none",
  "prov. > 15W",
  "prov. > 30W",
  "prov. > 45W",
  "uses < 1W",
  "uses < 3W",
  "uses < 6W",
  "uses < 10W"
};

const char * phy_delay_list[4] = {
  "<= 144ns",
  "unknown",
  "unknown",
  "unknown"
};


const char * format_list[5]= {
  "Format_0",
  "Format_1",
  "Format_2",
  "Format_6",
  "Format_7"
};

const char * format0_list[NUM_FORMAT0_MODES]= {
  "Format_0, Mode_0: 160x120 YUV (4:4:4)",
  "Format_0, Mode_1: 320x240 YUV (4:2:2)",
  "Format_0, Mode_2: 640x480 YUV (4:1:1)",
  "Format_0, Mode_3: 640x480 YUV (4:2:2)",
  "Format_0, Mode_4: 640x480 RGB 24bpp",
  "Format_0, Mode_5: 640x480 Mono 8bpp",
  "Format_0, Mode_6: 640x480 Mono 16bpp"
};

const char * format1_list[NUM_FORMAT1_MODES]= {
  "Format_1, Mode_0: 800x600 YUV (4:2:2)",
  "Format_1, Mode_1: 800x600 RGB 24bpp",
  "Format_1, Mode_2: 800x600 Mono 8bpp",
  "Format_1, Mode_3: 1024x768 YUV (4:2:2)",
  "Format_1, Mode_4: 1024x768 RGB 24bpp",
  "Format_1, Mode_5: 1024x768 Mono 8bpp",
  "Format_1, Mode_6: 800x600 Mono 16bpp",
  "Format_1, Mode_7: 1024x768 Mono 16bpp"
};

const char * format2_list[NUM_FORMAT2_MODES]= {
  "Format_2, Mode_0: 1280x960 YUV (4:2:2)",
  "Format_2, Mode_1: 1280x960 RGB 24bpp",
  "Format_2, Mode_2: 1280x960 Mono 8bpp",
  "Format_2, Mode_3: 1600x1200 YUV (4:2:2)",
  "Format_2, Mode_4: 1600x1200 RGB 24bpp",
  "Format_2, Mode_5: 1600x1200 Mono 8bpp",
  "Format_2, Mode_6: 1280x960 Mono 16bpp",
  "Format_2, Mode_7: 1600x1200 Mono 16bpp"
};

const char * format6_list[NUM_FORMAT6_MODES]= {
  "Mode_0: EXIF (not supported)"
};

const char * format7_list[NUM_MODE_FORMAT7]= {
  "Format7, Mode_0",
  "Format7, Mode_1",
  "Format7, Mode_2",
  "Format7, Mode_3",
  "Format7, Mode_4",
  "Format7, Mode_5",
  "Format7, Mode_6",
  "Format7, Mode_7"
};


const char *feature_menu_items_list[5]= {
  "OFF",
  "Man",
  "Auto",
  "Single",
  "Abs"
};

const char *help_key_bindings_keys[KEY_BINDINGS_NUM]= {
  "c",
  "n",
  "PAGE_UP",
  "PAGE_DOWN",
  ">",
  "<",
  "f",
  "m",
  "mouse button 2",
  "mouse button 1"
}; 

const char *help_key_bindings_functions[KEY_BINDINGS_NUM]= {
  "Set the WOI",
  "Set image zoom to normal (1:1)",
  "Zoom image by 10%",
  "Shrink image by 10%",
  "Zoom image by 100%",
  "Shrink image by 50%",
  "Toggle full-screen mode",
  "Switch to maximum size (Format7)",
  "Display the current pixel value & coordinates",
  "Select WOI region"
}; 
