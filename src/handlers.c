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

char text_date[] = "28", text_day[] = "Wed", text_time[] = "23z59", text_wx_age[] = "";
int battery_level = 100;
bool battery_charging = false, bt_connected = true, power_connected = false;
uint8_t lastday = 32;
Time last_time;

void handle_battery(BatteryChargeState charge_state) {
  battery_charging = charge_state.is_charging;
  power_connected = charge_state.is_plugged;
  battery_level = charge_state.charge_percent;

  // Redraw
  if(battery_layer) {
    layer_mark_dirty(battery_layer);
  }
}

void handle_bt(bool connected) {
  if (connected) { // this seems silly now, but vibrate notifications coming later TODO
    bt_connected = true;
  } else {
    bt_connected = false;
  }
  
  // Redraw
  if(bt_layer) {
    layer_mark_dirty(bt_layer);
  }
}

void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  last_time.hours = tick_time->tm_hour;
  last_time.hours -= (last_time.hours > 12) ? 12 : 0;
  last_time.minutes = tick_time->tm_min;
  last_time.seconds = tick_time->tm_sec;

  // Change day/date displays (if needed)
  if(lastday != tick_time->tm_mday) {
    lastday = tick_time->tm_mday;
    strftime(text_date, sizeof(text_date), "%d", tick_time);
    strftime(text_day, sizeof(text_day), "%a", tick_time);
  }
  
  if(conf.display_digital) {
    // Change digital time display (if enabled)
    if(conf.digital_as_zulu) {
      time_t temptime = mktime(tick_time);
      struct tm *zulu_time = gmtime(&temptime);
      strftime(text_time, sizeof(text_time), "%Hz%M", zulu_time);
    } else strftime(text_time, sizeof(text_time), "%R", tick_time);
  }

  // Redraw
  if(clock_layer) {
    layer_mark_dirty(clock_layer);
  }
}

void handle_app_timer_weather(void *dat) {
  bool res = trigger_weather();
  if(res) atwx = app_timer_register(1200000, handle_app_timer_weather, NULL); // Schedule next update
  else atwx = app_timer_register(300000, handle_app_timer_weather, NULL); // Schedule retry
}
