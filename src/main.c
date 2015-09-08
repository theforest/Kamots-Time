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

#define ANIMATION_DURATION 400
#define ANIMATION_DELAY    500
AppTimer *atwx = NULL;

static void unload() {
  // Save any configuration changes
  int confbytes = 0;
  confbytes = persist_write_data(KEY_CONFDAT, &conf, sizeof(conf)); // save config
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote config ver %i, bytes: %i", confver, confbytes);

  // Unsubscribe from events
  app_timer_cancel(atwx);
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_stack_remove(main_window, true);
  window_destroy(main_window);
  clock_layer = NULL;
  battery_layer = NULL;
  bt_layer = NULL;
  weather_t_layer = NULL;
  weather_ft_layer = NULL;
  weather_a_layer = NULL;
  date_layer = NULL;
  day_layer = NULL;
  digitime_layer = NULL;
  weather_c_layer = NULL;
  background_layer = NULL;
}

static void load() {
  // Setup window
  main_window = window_create();
  window_set_background_color(main_window, conf.color_watchface_background);
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
    .appear = window_appear,
  });
  window_stack_push(main_window, true);

  // Subscribe to events
  battery_state_service_subscribe(handle_battery);
  if(conf.display_bt_status) bluetooth_connection_service_subscribe(handle_bt);
  if(conf.display_second_hand) tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  else tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Timers
  if(conf.display_weather) atwx = app_timer_register(3000, handle_app_timer_weather, NULL);
}

void reload() {
  unload();
  load();
  if(clock_layer) {
    layer_mark_dirty(clock_layer);
  }
  if(battery_layer) {
    layer_mark_dirty(battery_layer);
  }
  if(bt_layer) {
    layer_mark_dirty(bt_layer);
  }
  if(weather_c_layer) {
    layer_mark_dirty(weather_c_layer);
  }
  if(date_layer) {
    layer_mark_dirty(text_layer_get_layer(date_layer));
  }
  if(day_layer) {
    layer_mark_dirty(text_layer_get_layer(day_layer));
  }
  if(digitime_layer) {
    layer_mark_dirty(text_layer_get_layer(digitime_layer));
  }
  if(weather_t_layer) {
    layer_mark_dirty(text_layer_get_layer(weather_t_layer));
  }
  if(weather_ft_layer) {
    layer_mark_dirty(text_layer_get_layer(weather_ft_layer));
  }
  if(background_layer) {
    layer_mark_dirty(text_layer_get_layer(background_layer));
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

  if(conf.weather_temp_format) strncpy(text_wx_ft, "F", 1);
  else strncpy(text_wx_ft, "C", 1);

  load(); // load main watchface

  // Subscribe to events
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);

  // Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, OUTBOX_SIZE);

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
  // Unsubscribe from events
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  
  unload(); // unload main watchface
}

int main() {
  init();
  app_event_loop();
  deinit();
}
