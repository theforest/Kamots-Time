#pragma once
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
#include <pebble.h>
#ifndef MAIN_H
#define MAIN_H
  
typedef struct { // MUST change CURRENT_CONFVER in phonecomm.c if this changes
  GColor color_hour_hand;
  GColor color_minute_hand;
  GColor color_hour_markers;
  GColor color_watchface_background;
  GColor color_watchface_outline;
  GColor color_surround_background;
  bool display_digital;
  int8_t hour_markers_count; // conf v3
  bool display_bt_status; // conf v3
  GColor color_second_hand; // conf v4
  bool display_second_hand; // conf v4
  bool digital_as_zulu; // conf v4
  bool display_weather; // conf v5
// } __attribute__((__packed__)) appConfig;  // if needed as config grows
} appConfig;

typedef struct {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
} Time;

typedef struct {
  uint8_t conditions;
  float temperature;
  uint32_t timestamp;
} Weather;

// functions
extern void clock_update_proc(Layer *layer, GContext *ctx);
extern void battery_update_proc(Layer *layer, GContext *ctx);
extern void bt_update_proc(Layer *layer, GContext *ctx);
extern void handle_battery(BatteryChargeState charge_state);
extern appConfig load_defaults();
extern void handle_battery(BatteryChargeState charge_state);
extern void handle_bt(bool connected);
extern void tick_handler(struct tm *tick_time, TimeUnits changed);
extern void convertconfig();
extern void window_unload(Window *window);
extern void window_load(Window *window);
extern void window_appear(Window *window);
extern void inbox_dropped_callback(AppMessageResult reason, void *context);
extern void inbox_received_callback(DictionaryIterator *iterator, void *context);
extern void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers);
extern void radius_update(Animation *anim, AnimationProgress dist_normalized);
extern void hands_update(Animation *anim, AnimationProgress dist_normalized);

// variables
extern Window *main_window;
extern Layer *clock_layer, *battery_layer, *bt_layer;
extern TextLayer *date_layer, *day_layer, *digitime_layer, *background_layer, *weather_t_layer, *weather_c_layer;
extern appConfig conf;
extern int confver;
extern Time last_time;
extern Time anim_time;
extern bool animating;
extern char text_date[], text_day[], text_time[], text_wx_c[], text_wx_t[];
extern int battery_level;
extern bool battery_charging, bt_connected, power_connected;
extern GPath *path_bolt_ptr, *path_bt_ptr, *path_plug_ptr;
extern int radius;
extern GPoint center;
extern const uint32_t KEY_CONFVER;
extern const uint32_t KEY_CONFDAT;
extern const uint8_t CURRENT_CONFVER;
extern uint8_t config_changed;
extern void reload();
#endif
