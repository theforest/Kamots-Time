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
#include "gpath.h"

Window *main_window;
Layer *clock_layer, *battery_layer, *bt_layer, *weather_c_layer;
TextLayer *date_layer, *day_layer, *digitime_layer, *background_layer, *weather_t_layer, *weather_ft_layer, *weather_a_layer;
GPoint center;

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

  // Weather layers
  if(conf.display_weather) {
    path_lcloud_ptr = gpath_create(&WX_LCLOUD_PATH_INFO); // Create large cloud symbol
    gpath_move_to(path_lcloud_ptr, GPoint(4, 5));
    path_scloud_ptr = gpath_create(&WX_SCLOUD_PATH_INFO); // Create small cloud symbol
    gpath_move_to(path_scloud_ptr, GPoint(19, 2));
    path_lightning_ptr = gpath_create(&WX_LIGHTNING_PATH_INFO); // Create lightning symbol
    gpath_move_to(path_lightning_ptr, GPoint(11, 14));
    path_snowflake_ptr = gpath_create(&WX_SNOWFLAKE_PATH_INFO); // Create snowflake symbol

    weather_t_layer = text_layer_create(GRect(0, 148, 38, 19)); // Bottom left of screen
    text_layer_set_text(weather_t_layer, text_wx_t);
    text_layer_set_background_color(weather_t_layer, conf.color_surround_background);
    text_layer_set_text_color(weather_t_layer, conf.color_watchface_outline);
    text_layer_set_text_alignment(weather_t_layer, GTextAlignmentRight);
    text_layer_set_font(weather_t_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(weather_t_layer));

    weather_ft_layer = text_layer_create(GRect(38, 152, 8, 16)); // Bottom left of screen
    text_layer_set_text(weather_ft_layer, text_wx_ft);
    text_layer_set_background_color(weather_ft_layer, conf.color_surround_background);
    text_layer_set_text_color(weather_ft_layer, conf.color_watchface_outline);
    text_layer_set_text_alignment(weather_ft_layer, GTextAlignmentLeft);
    text_layer_set_font(weather_ft_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_layer, text_layer_get_layer(weather_ft_layer));
    
    weather_a_layer = text_layer_create(GRect(0, 138, 28, 16)); // Bottom left of screen
    text_layer_set_text(weather_a_layer, text_wx_age);
    text_layer_set_background_color(weather_a_layer, conf.color_surround_background);
    text_layer_set_text_color(weather_a_layer, conf.color_watchface_outline);
    text_layer_set_text_alignment(weather_a_layer, GTextAlignmentLeft);
    text_layer_set_font(weather_a_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_layer, text_layer_get_layer(weather_a_layer));

    weather_c_layer = layer_create(GRect(112, 140, 36, 27)); // Bottom right of screen
    layer_set_update_proc(weather_c_layer, weather_update_proc);
    layer_add_child(window_layer, weather_c_layer);
  }

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
  gpath_destroy(path_lcloud_ptr);
  gpath_destroy(path_scloud_ptr);
  gpath_destroy(path_lightning_ptr);
  gpath_destroy(path_snowflake_ptr);
  text_layer_destroy(date_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(digitime_layer);
  text_layer_destroy(background_layer);
  text_layer_destroy(weather_t_layer);
  text_layer_destroy(weather_ft_layer);
  text_layer_destroy(weather_a_layer);
  layer_destroy(clock_layer);
  layer_destroy(battery_layer);
  layer_destroy(bt_layer);
  layer_destroy(weather_c_layer);
}
