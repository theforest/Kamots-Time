/*
*** Kamots Time ***
Original code based from: https://github.com/pebble-examples/ks-clock-face/commit/38d1daa29ab4354f592ac274206b95ba648bec63
...which is Copyright (C) 2015 Pebble Technology and licensed under the MIT License

Changes and additions by kamotswind...
Copyright 2015 kamotswind

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions and
limitations under the License.
*/

#include "main.h"

GPath *path_bolt_ptr = NULL;
GPath *path_bt_ptr = NULL;
GPath *path_plug_ptr = NULL;
GPoint center;

const GPathInfo BOLT_PATH_INFO = {
  .num_points = 6,
  .points = (GPoint []) {
    {(float)4.5, 0},
    {(float)4.5, (float)13.95},
    {8, (float)13.95},
    {(float)2.25, 26},
    {(float)4.5, 16},
    {0, 16}
  }
};

const GPathInfo BT_PATH_INFO = {
  .num_points = 8,
  .points = (GPoint []) {
    {0, 0},
    {3, 3},
    {-3, 9},
    {0, 6},
    {-3, 3},
    {3, 9},
    {0, 12},
    {0, 6}
  }
};

const GPathInfo PLUG_PATH_INFO = {
  .num_points = 20,
  .points = (GPoint []) {
    {0, 3},
    {2, 3},
    {2, 0},
    {3, 0},
    {3, 3},
    {6, 3},
    {6, 0},
    {7, 0},
    {7, 3},
    {9, 3},
    {9, 7},
    {8, 7},
    {8, 8},
    {5, 8},
    {5, 12},
    {4, 12},
    {4, 8},
    {1, 8},
    {1, 7},
    {0, 7}
  }
};

void window_appear(Window *window) {
  window_set_background_color(window, conf.color_watchface_background);
}

void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  path_bolt_ptr = gpath_create(&BOLT_PATH_INFO); // Create charging indicator bolt
  gpath_rotate_to(path_bolt_ptr, TRIG_MAX_ANGLE / 360 * 15);
  gpath_move_to(path_bolt_ptr, GPoint(6, -9));

  path_plug_ptr = gpath_create(&PLUG_PATH_INFO); // Create plugged in indicator

  // background layer (hopefully someday window background color will work and this can go away)
  background_layer = text_layer_create(window_bounds); // Entire screen
  text_layer_set_text(background_layer, " ");
  text_layer_set_background_color(background_layer, conf.color_surround_background);
  text_layer_set_text_color(background_layer, conf.color_watchface_outline);
  layer_add_child(window_layer, text_layer_get_layer(background_layer));

  // digital time
  if(conf.display_digital) {
    digitime_layer = text_layer_create(GRect(52, 148, 40, 19)); // Bottom center of screen
    text_layer_set_text(digitime_layer, text_time);
    text_layer_set_background_color(digitime_layer, conf.color_surround_background);
    text_layer_set_text_color(digitime_layer, conf.color_watchface_outline);
    text_layer_set_text_alignment(digitime_layer, GTextAlignmentCenter);
    text_layer_set_font(digitime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(digitime_layer));
  }
  
  // Main clock layer
  clock_layer = layer_create(GRect(-1, 12, 144, 142));
  layer_set_update_proc(clock_layer, clock_update_proc);
  layer_add_child(window_layer, clock_layer);
  GRect clock_bounds = layer_get_bounds(clock_layer);
  center = grect_center_point(&clock_bounds);

  // Battery status layer
  battery_layer = layer_create(GRect(106, 0, 38, 12)); // Top right-hand of screen, 38x12 size
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);
  
  // Bluetooth status layer
  if(conf.display_bt_status) {
    path_bt_ptr = gpath_create(&BT_PATH_INFO); // Create bluetooth indicator symbol
    gpath_move_to(path_bt_ptr, GPoint(3, 0));
    bt_layer = layer_create(GRect(134, 14, 7, 13)); // Top right-hand of screen, 7x13 size
    layer_set_update_proc(bt_layer, bt_update_proc);
    layer_add_child(window_layer, bt_layer);
    if (bluetooth_connection_service_peek()) bt_connected = true;
    else bt_connected = false;
  }

  // day layer
  day_layer = text_layer_create(GRect(2, -4, 30, 18)); // Top left-hand of screen, 30x18 size
  text_layer_set_text(day_layer, text_day);
  text_layer_set_background_color(day_layer, conf.color_surround_background);
  text_layer_set_text_color(day_layer, conf.color_watchface_outline);
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(day_layer));
  
  // date layer
  date_layer = text_layer_create(GRect(32, -4, 18, 18)); // Top left-hand of screen, 18x18 size
  text_layer_set_text(date_layer, text_date);
  text_layer_set_background_color(date_layer, conf.color_surround_background);
  text_layer_set_text_color(date_layer, conf.color_watchface_outline);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

void window_unload(Window *window) {
  gpath_destroy(path_bolt_ptr);
  gpath_destroy(path_bt_ptr);
  gpath_destroy(path_plug_ptr);
  text_layer_destroy(date_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(digitime_layer);
  text_layer_destroy(background_layer);
  layer_destroy(clock_layer);
  layer_destroy(battery_layer);
  layer_destroy(bt_layer);
}