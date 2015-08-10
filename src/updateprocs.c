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
#include "util.h"

#define HAND_MARGIN  14

void clock_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, conf.color_surround_background);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

#ifdef PBL_COLOR
  // For smooth lines
  graphics_context_set_antialiased(ctx, true);
#endif
  
  // clockface
  graphics_context_set_fill_color(ctx, conf.color_watchface_background);
  graphics_fill_circle(ctx, center, radius);

  // Draw outline
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
#ifdef PBL_COLOR
  graphics_context_set_stroke_width(ctx, 2);
#endif
  graphics_draw_circle(ctx, center, radius);
#ifndef PBL_COLOR
  graphics_draw_circle(ctx, center, radius-1); // draw an extra circle on B&W Pebble.
#endif

  // Don't use current time while animating
  Time mode_time = (animating) ? anim_time : last_time;

  // Adjust for minutes through the hour
  float second_angle = 0;
  if(conf.display_second_hand) { second_angle = TRIG_MAX_ANGLE * mode_time.seconds / 60; }
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
    handle_battery(battery_state_service_peek());
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint second_hand = (GPoint) { .x = (int16_t)0, .y = (int16_t)0 };
  if(conf.display_second_hand) {
    second_hand = (GPoint) {
      .x = (int16_t)(sin_lookup(second_angle) * (int32_t)(radius - HAND_MARGIN) / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)(radius - HAND_MARGIN) / TRIG_MAX_RATIO) + center.y,
    };
  }
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)(radius - HAND_MARGIN) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)(radius - HAND_MARGIN) / TRIG_MAX_RATIO) + center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + center.y,
  };

  // Draw hands with positive length only
  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_stroke_color(ctx, conf.color_minute_hand);
  if(radius > HAND_MARGIN) {
    #ifdef PBL_COLOR
    graphics_draw_line(ctx, center, minute_hand);
    #else
    graphics_draw_line2(ctx, center, minute_hand, 4);
    #endif
  }
  graphics_context_set_stroke_color(ctx, conf.color_hour_hand);
  if(radius > 2 * HAND_MARGIN) {
    #ifdef PBL_COLOR
    graphics_draw_line(ctx, center, hour_hand);
    #else
    graphics_draw_line2(ctx, center, hour_hand, 6);
    #endif
  }
  if(conf.display_second_hand) {
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, conf.color_second_hand);
    if(radius > 2 * HAND_MARGIN) {
      graphics_draw_line(ctx, center, second_hand);
    }
  }

  // Draw hour markers
  float marker_angle;
  GPoint hour_marker;
  graphics_context_set_fill_color(ctx, conf.color_hour_markers);
  for(int8_t i = 12; i>0; i--) {
    marker_angle = TRIG_MAX_ANGLE * i / 12;
    hour_marker = (GPoint) {
    .x = (int16_t)(sin_lookup(marker_angle) * (int32_t)(radius - 7) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(marker_angle) * (int32_t)(radius - 7) / TRIG_MAX_RATIO) + center.y,};
    if((conf.hour_markers_count == 12) || ((conf.hour_markers_count == 1) && (i == 12)) ||
      ((conf.hour_markers_count == 4) && (i==12 || i==9 || i==6 || i==3))){
      graphics_fill_circle(ctx, hour_marker, 4);    
    }
  }
}

void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
  graphics_context_set_fill_color(ctx, conf.color_watchface_outline);
  graphics_draw_round_rect(ctx, GRect(12, 3, 20, 9), 1); // Outside of battery
  graphics_fill_rect(ctx, GRect(32, 6, 2, 3), 0, 0); // Battery positive terminal
  graphics_fill_rect(ctx, GRect(14, 5, map(battery_level, 0, 100, 1, 16), 5), 0, 0); // Inside of battery
  if(battery_charging) {
    gpath_draw_filled(ctx, path_bolt_ptr); // lightning bolt
  } else if(power_connected) {
    gpath_draw_outline(ctx, path_plug_ptr); // power plug
    graphics_fill_rect(ctx, GRect(1, 4, 8, 4), 0, 0);
  } else if(battery_level <= 20) {
    graphics_fill_rect(ctx, GRect(7, 2, 2, 6), 0, 0); // exclamation mark
    graphics_fill_rect(ctx, GRect(7, 10, 2, 2), 0, 0);
  }
}

void bt_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_width(ctx, 1);
  if(bt_connected) {
    graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
    gpath_draw_outline(ctx, path_bt_ptr);  // bluetooth symbol
  } else {
    graphics_context_set_stroke_color(ctx, conf.color_surround_background);
    gpath_draw_outline(ctx, path_bt_ptr);  // bluetooth symbol    
  }
}