/*
*** Kamots Time ***
Original code based from: https://github.com/pebble-examples/ks-clock-face
...which is Copyright (C) 2015 Pebble Technology and licensed under the MIT License

Changes and additions by kamotswind...
Copyright 2013 kamotswind

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions and
limitations under the License.
*/

#include <pebble.h>
#include "main.h"

#define COLORS       true
#define ANTIALIASING true

#define HAND_MARGIN  14
#define FINAL_RADIUS 68

#define ANIMATION_DURATION 300
#define ANIMATION_DELAY    400

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer, *s_battery_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0;
static bool s_animating = false;
int battery_level = 100;
bool battery_charging = false;
GColor color_hour_hand, color_minute_hand, color_hour_markers,
       color_watchface_outline, color_watchface_background, color_surround_background;

int map(int x, int in_min, int in_max, int out_min, int out_max) { // Borrowed from Arduino
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void handle_battery(BatteryChargeState charge_state) {
  battery_charging = charge_state.is_charging;
  battery_level = charge_state.charge_percent;

  // Redraw
  if(s_battery_layer) {
    layer_mark_dirty(s_battery_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void main_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, color_surround_background);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

  // For smooth lines
  graphics_context_set_antialiased(ctx, ANTIALIASING);

  // clockface
  graphics_context_set_fill_color(ctx, color_watchface_background);
  graphics_fill_circle(ctx, s_center, s_radius);

  // Draw outline
  graphics_context_set_stroke_color(ctx, color_watchface_outline);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, s_center, s_radius);

  // Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
    handle_battery(battery_state_service_peek());
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
  };

  // Draw hands with positive length only
  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_stroke_color(ctx, color_hour_hand);
  if(s_radius > 2 * HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, hour_hand);
  } 
  graphics_context_set_stroke_color(ctx, color_minute_hand);
  if(s_radius > HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, minute_hand);
  }
  
  // Draw hour markers
  float marker_angle;
  GPoint hour_marker;
  graphics_context_set_fill_color(ctx, color_hour_markers);
  for(int8_t i = 12; i>0; i--) {
    marker_angle = TRIG_MAX_ANGLE * i / 12;
    hour_marker = (GPoint) {
    .x = (int16_t)(sin_lookup(marker_angle) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(marker_angle) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.y,};
    graphics_fill_circle(ctx, hour_marker, 4);
  }
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, GRect(0, 0, 20, 9), 1); // Outside of battery
  graphics_fill_rect(ctx, GRect(20, 3, 2, 3), 0, 0); // Battery positive terminal
  if(battery_level <= 20) graphics_context_set_fill_color(ctx, battery_charging ? GColorBlue:GColorDarkCandyAppleRed);
  else graphics_context_set_fill_color(ctx, battery_charging ? GColorBlue:GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, map(battery_level, 0, 100, 0, 16), 5), 0, 0); // Inside of battery
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);
  
  // Main watchface layer
  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, main_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  // Battery status layer
  s_battery_layer = layer_create(GRect(120, 2, 24, 10)); // Top right-hand of screen, 24x10 size
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);
}

static void window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_battery_layer);
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void init() {
  color_hour_hand = GColorRed; // Default colors
  color_minute_hand = GColorBlue;
  color_hour_markers = GColorDarkGreen;
  color_watchface_outline = GColorBlack;
  color_watchface_background = GColorWhite;
  color_surround_background = GColorDarkGreen;
  
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to events
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);

  // Prepare animations
  AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
