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

static Window *s_main_window;
static Layer *s_clock_layer, *s_battery_layer;
static TextLayer *s_date_layer, *s_day_layer, *s_digitime_layer, *s_background_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0;
static bool s_animating = false;
int battery_level = 100;
bool battery_charging = false;
char text_date[] = "28", text_day[] = "Wed", text_time[] = "23:59";
int confver = 0;
appConfig conf;

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

static appConfig load_defaults() { // fill the default configuration values
  appConfig defaultconf;
  #ifndef PBL_COLOR
  defaultconf.color_hour_hand = GColorBlack; // Default colors, well, black and white
  defaultconf.color_minute_hand = GColorBlack;
  defaultconf.color_hour_markers = GColorBlack;
  defaultconf.color_watchface_background = GColorWhite;
  defaultconf.color_watchface_outline = GColorBlack;
  defaultconf.color_surround_background = GColorClear;
  defaultconf.display_digital = true; // Default DO displaying digital time
  #else
  defaultconf.color_hour_hand = GColorRed; // Default colors
  defaultconf.color_minute_hand = GColorBlue;
  defaultconf.color_hour_markers = GColorDarkGreen;
  defaultconf.color_watchface_background = GColorWhite;
  defaultconf.color_watchface_outline = GColorBlack;
  defaultconf.color_surround_background = GColorDarkGreen;
  defaultconf.display_digital = false; // Default not displaying digital time
  #endif
  defaultconf.hour_markers_count = 12; // Show all hour markers by default
  defaultconf.display_bt_status = false; // Default not displaying bluetooth status
  return defaultconf;
}

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

  // Change day/date displays
  strftime(text_date, sizeof(text_date), "%d", tick_time);
  strftime(text_day, sizeof(text_day), "%a", tick_time);
  
  if(conf.display_digital) {
    // Change digital time display (if enabled)
    strftime(text_time, sizeof(text_time), "%H:%M", tick_time);
  }
    
  
  // Redraw
  if(s_clock_layer) {
    layer_mark_dirty(s_clock_layer);
  }
  if(s_date_layer) {
    layer_mark_dirty(text_layer_get_layer(s_date_layer));
  }
  if(s_day_layer) {
    layer_mark_dirty(text_layer_get_layer(s_day_layer));
  }
  if(s_digitime_layer) {
    layer_mark_dirty(text_layer_get_layer(s_digitime_layer));
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void main_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, conf.color_surround_background);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

#ifdef PBL_COLOR
  // For smooth lines
  graphics_context_set_antialiased(ctx, ANTIALIASING);
#endif
  
  // clockface
  graphics_context_set_fill_color(ctx, conf.color_watchface_background);
  graphics_fill_circle(ctx, s_center, s_radius);

  // Draw outline
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
#ifdef PBL_COLOR
  graphics_context_set_stroke_width(ctx, 2);
#endif
  graphics_draw_circle(ctx, s_center, s_radius);
#ifndef PBL_COLOR
  graphics_draw_circle(ctx, s_center, s_radius-1); // draw an extra circle on B&W Pebble.
#endif

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
  graphics_context_set_stroke_color(ctx, conf.color_minute_hand);
  if(s_radius > HAND_MARGIN) {
    #ifdef PBL_COLOR
    graphics_draw_line(ctx, s_center, minute_hand);
    #else
    graphics_draw_line2(ctx, s_center, minute_hand, 4);
    #endif
  }
  graphics_context_set_stroke_color(ctx, conf.color_hour_hand);
  if(s_radius > 2 * HAND_MARGIN) {
    #ifdef PBL_COLOR
    graphics_draw_line(ctx, s_center, hour_hand);
    #else
    graphics_draw_line2(ctx, s_center, hour_hand, 6);
    #endif
  } 
  
  // Draw hour markers
  float marker_angle;
  GPoint hour_marker;
  graphics_context_set_fill_color(ctx, conf.color_hour_markers);
  for(int8_t i = 12; i>0; i--) {
    marker_angle = TRIG_MAX_ANGLE * i / 12;
    hour_marker = (GPoint) {
    .x = (int16_t)(sin_lookup(marker_angle) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(marker_angle) * (int32_t)(s_radius - 7) / TRIG_MAX_RATIO) + s_center.y,};
    if((conf.hour_markers_count == 12) || ((conf.hour_markers_count == 1) && (i == 12)) ||
      ((conf.hour_markers_count == 4) && (i==12 || i==9 || i==6 || i==3))){
      graphics_fill_circle(ctx, hour_marker, 4);    
    }
  }
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
  graphics_context_set_fill_color(ctx, conf.color_watchface_outline);
  graphics_draw_round_rect(ctx, GRect(12, 3, 20, 9), 1); // Outside of battery
  graphics_fill_rect(ctx, GRect(32, 6, 2, 3), 0, 0); // Battery positive terminal
  graphics_fill_rect(ctx, GRect(14, 5, map(battery_level, 0, 100, 1, 16), 5), 0, 0); // Inside of battery
  if(battery_charging) {
    gpath_draw_filled(ctx, s_path_bolt_ptr); // lightning bolt
  } else if(battery_level <= 20) {
    graphics_fill_rect(ctx, GRect(7, 2, 2, 6), 0, 0); // exclamation mark
    graphics_fill_rect(ctx, GRect(7, 10, 2, 2), 0, 0);
  }
}

static void window_appear(Window *window) {
  window_set_background_color(window, conf.color_watchface_background);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_path_bolt_ptr = gpath_create(&BOLT_PATH_INFO); // Create charging indicator bolt
  gpath_rotate_to(s_path_bolt_ptr, TRIG_MAX_ANGLE / 360 * 15);
  gpath_move_to(s_path_bolt_ptr, GPoint(6, -9));

  // background layer
  s_background_layer = text_layer_create(window_bounds); // Entire screen
  text_layer_set_text(s_background_layer, " ");
  text_layer_set_background_color(s_background_layer, conf.color_surround_background);
  text_layer_set_text_color(s_background_layer, conf.color_watchface_outline);
  layer_add_child(window_layer, text_layer_get_layer(s_background_layer));

  // digital time
  if(conf.display_digital) {
    s_digitime_layer = text_layer_create(GRect(52, 148, 40, 19)); // Bottom center of screen
    text_layer_set_text(s_digitime_layer, text_time);
    text_layer_set_background_color(s_digitime_layer, conf.color_surround_background);
    text_layer_set_text_color(s_digitime_layer, conf.color_watchface_outline);
    text_layer_set_text_alignment(s_digitime_layer, GTextAlignmentCenter);
    text_layer_set_font(s_digitime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_digitime_layer));
  }
  
  // Main clock layer
  s_clock_layer = layer_create(GRect(-1, 12, 144, 142));
  layer_set_update_proc(s_clock_layer, main_update_proc);
  layer_add_child(window_layer, s_clock_layer);
  GRect clock_bounds = layer_get_bounds(s_clock_layer);
  s_center = grect_center_point(&clock_bounds);

  // Battery status layer
  s_battery_layer = layer_create(GRect(106, 0, 38, 12)); // Top right-hand of screen, 38x12 size
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);

  // day layer
  s_day_layer = text_layer_create(GRect(2, -4, 30, 18)); // Top left-hand of screen, 30x18 size
  text_layer_set_text(s_day_layer, text_day);
  text_layer_set_background_color(s_day_layer, conf.color_surround_background);
  text_layer_set_text_color(s_day_layer, conf.color_watchface_outline);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  
  // date layer
  s_date_layer = text_layer_create(GRect(32, -4, 18, 18)); // Top left-hand of screen, 18x18 size
  text_layer_set_text(s_date_layer, text_date);
  text_layer_set_background_color(s_date_layer, conf.color_surround_background);
  text_layer_set_text_color(s_date_layer, conf.color_watchface_outline);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_digitime_layer);
  text_layer_destroy(s_background_layer);
  layer_destroy(s_clock_layer);
  layer_destroy(s_battery_layer);
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_clock_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_clock_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received %d bytes", (unsigned int)dict_size(iterator));
  int8_t hm_count = 0;
#ifdef PBL_COLOR
  int32_t colorint = 0;
#endif

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {

    // Which key was received?
    switch(t->key) {
    case D_DT:
      if (t->value->int8 == 1) {
        conf.display_digital = true;
      } else {
        conf.display_digital = false;
      }
      break;
#ifdef PBL_COLOR
    case C_HH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_hour_hand = GColorFromHEX(colorint);
      break;

    case C_MH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_minute_hand = GColorFromHEX(colorint);
      break;

    case C_HM:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_hour_markers = GColorFromHEX(colorint);
      break;

    case C_WB:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_watchface_background = GColorFromHEX(colorint);
      break;

    case C_WO:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_watchface_outline = GColorFromHEX(colorint);
      break;

    case C_SB:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_surround_background = GColorFromHEX(colorint);
      break;
#else
    case C_HH:
    case C_MH:
    case C_HM:
    case C_WB:
    case C_WO:
    case C_SB:
      // nothing to do
      break;
#endif
    case HM_C:
      hm_count = t->value->int8;
      if(hm_count == 12) conf.hour_markers_count = 12;
      if(hm_count == 4) conf.hour_markers_count = 4;
      if(hm_count == 1) conf.hour_markers_count = 1;
      break;
      
    case D_BT:
      if (t->value->int8 == 1) {
        conf.display_bt_status = true;
      } else {
        conf.display_bt_status = false;
      }
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped! Err: %d", reason);
} 

static void convertconfig() {
  int confbytes = 0;
  appConfig newconf = load_defaults();
  appConfig defaultconf = newconf;
  confbytes = persist_read_data(KEY_CONFDAT, &conf, sizeof(conf)); // load saved config
  if(confbytes <= 1) { // this should never happen, but just in case...
    APP_LOG(APP_LOG_LEVEL_WARNING, "Read config ver %i, bytes: %i corrupt! Load defaults...", confver, confbytes);
    conf = load_defaults(); // load defaults because config was corrupt
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Read old config ver %i, bytes: %i", confver, confbytes);
    newconf = conf;
    switch(confver) {
      case 1:
        newconf.color_watchface_background = conf.color_watchface_outline;
        newconf.color_watchface_outline = conf.color_watchface_background;
        // no break here, continue with conversion

      case 2:
        newconf.hour_markers_count = defaultconf.hour_markers_count;
        newconf.display_bt_status = defaultconf.display_bt_status;
        break;
      
      default:
        newconf = defaultconf;
    }
    confver = CURRENT_CONFVER; // set config version to current version after conversion
    persist_write_int(KEY_CONFVER, confver); // save new config version
    confbytes = persist_write_data(KEY_CONFDAT, &newconf, sizeof(newconf)); // save new converted config
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote converted config ver %i, bytes: %i", confver, confbytes);    
  }
}

static void init() {
  // Check if we have stored configuration data, otherwise set defaults
  int confbytes = 0;
  if(persist_exists(KEY_CONFVER)) confver = persist_read_int(KEY_CONFVER); // current config version
  else {
    confver = CURRENT_CONFVER;
    persist_write_int(KEY_CONFVER, confver); // config version didn't exist, write one
  }
  if(persist_exists(KEY_CONFDAT)) { // See if there already is configuration data
    if(confver < CURRENT_CONFVER) convertconfig();
    confbytes = persist_read_data(KEY_CONFDAT, &conf, sizeof(conf)); // load saved config
    if(confbytes != (int)sizeof(conf)) { // this should never happen, but just in case...
      APP_LOG(APP_LOG_LEVEL_WARNING, "Read config ver %i, bytes: %i ! Load defaults...", confver, confbytes);
      conf = load_defaults(); // load defaults because config was corrupt
    } else APP_LOG(APP_LOG_LEVEL_DEBUG, "Read config ver %i, bytes: %i", confver, confbytes);
  } else {
    conf = load_defaults(); // load defaults because we have no existing configuration
    confbytes = persist_write_data(KEY_CONFDAT, &conf, sizeof(conf)); // save config
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote default config ver %i, bytes: %i", confver, confbytes);
  }

  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_background_color(s_main_window, conf.color_watchface_background);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
    .appear = window_appear,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to events
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  
  // Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, 16);

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
  // Save any configuration changes
  int confbytes = 0;
  confbytes = persist_write_data(KEY_CONFDAT, &conf, sizeof(conf)); // save config
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Exiting! Wrote config ver %i, bytes: %i", confver, confbytes);
  
  // Unsubscribe from events
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
