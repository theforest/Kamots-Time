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
time_t nextwx = 0;

void clock_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, conf.color_surround_background);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

//#ifndef PBL_PLATFORM_APLITE
  // For smooth lines
  graphics_context_set_antialiased(ctx, true);
//#endif
  
  // clockface
  graphics_context_set_fill_color(ctx, conf.color_watchface_background);
  graphics_fill_circle(ctx, center, radius);

  // Draw outline
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, radius);

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
    graphics_draw_line(ctx, center, minute_hand);
  }
  graphics_context_set_stroke_color(ctx, conf.color_hour_hand);
  if(radius > 2 * HAND_MARGIN) {
    graphics_draw_line(ctx, center, hour_hand);
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

  weather_calc_age(); // Update weather age as clock updates
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

void draw_left_rain(GContext *ctx) {
  graphics_fill_rect(ctx, GRect(4, 18, 2, 3), 0, 0); // Rain, left
  graphics_fill_rect(ctx, GRect(8, 18, 2, 3), 0, 0); // Rain, left
  graphics_fill_rect(ctx, GRect(2, 22, 2, 3), 0, 0); // Rain, left
  graphics_fill_rect(ctx, GRect(6, 22, 2, 3), 0, 0); // Rain, left
}

void draw_center_rain(GContext *ctx) {
  graphics_fill_rect(ctx, GRect(12, 18, 2, 3), 0, 0); // Rain, center
  graphics_fill_rect(ctx, GRect(10, 22, 2, 3), 0, 0); // Rain, center

}

void draw_right_rain(GContext *ctx) {
  graphics_fill_rect(ctx, GRect(16, 18, 2, 3), 0, 0); // Rain, right
  graphics_fill_rect(ctx, GRect(20, 18, 2, 3), 0, 0); // Rain, right
  graphics_fill_rect(ctx, GRect(14, 22, 2, 3), 0, 0); // Rain, right
  graphics_fill_rect(ctx, GRect(18, 22, 2, 3), 0, 0); // Rain, right
}


void draw_large_cloud(GContext *ctx) {
  gpath_draw_filled(ctx, path_lcloud_ptr); // large cloud
}

void draw_small_cloud(GContext *ctx) {
  gpath_draw_filled(ctx, path_scloud_ptr); // small cloud
}

void draw_large_sunmoon(GContext *ctx) {
  graphics_fill_circle(ctx, GPoint(14,12), 10); // large sun
  if(wx.conditions > 100) {
    graphics_context_set_fill_color(ctx, conf.color_surround_background);
    graphics_fill_circle(ctx, GPoint(7,12), 10); // make sun in to moon
    graphics_context_set_fill_color(ctx, conf.color_watchface_outline);
  }
}

void draw_small_sunmoon(GContext *ctx) {
  graphics_fill_circle(ctx, GPoint(25,5), 4); // small sun/moon
  if(wx.conditions > 100) {
    graphics_context_set_fill_color(ctx, conf.color_surround_background);
    graphics_fill_circle(ctx, GPoint(21,5), 4); // make sun in to moon
    graphics_context_set_fill_color(ctx, conf.color_watchface_outline);
  }
}

void weather_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, conf.color_watchface_outline);
  graphics_context_set_fill_color(ctx, conf.color_watchface_outline);

  switch(wx.conditions) {
    case 1:
    case 101:
      draw_large_sunmoon(ctx);
      break;
    case 2:
    case 102:
      draw_small_sunmoon(ctx);
    case 3:
    case 103:
      draw_large_cloud(ctx);
      break;
    case 4:
    case 104:
      draw_large_cloud(ctx);
      draw_small_cloud(ctx);
      break;
    case 9:
    case 109:
      draw_large_cloud(ctx);
      draw_small_cloud(ctx);
      draw_left_rain(ctx);
      draw_right_rain(ctx);
      break;
    case 10:
    case 110:
      draw_small_sunmoon(ctx);
      draw_large_cloud(ctx);
      draw_left_rain(ctx);
      draw_right_rain(ctx);
      break;
    case 11:
    case 111:
      draw_large_cloud(ctx);
      draw_small_cloud(ctx);
      draw_left_rain(ctx);
      draw_center_rain(ctx);
      gpath_draw_filled(ctx, path_lightning_ptr); // lightning
      break;
    case 13:
    case 113:
      draw_large_cloud(ctx);
      draw_small_cloud(ctx);
      gpath_move_to(path_snowflake_ptr, GPoint(6, 18));
      gpath_draw_outline(ctx, path_snowflake_ptr); // snowflake
      gpath_move_to(path_snowflake_ptr, GPoint(15, 19));
      gpath_draw_outline(ctx, path_snowflake_ptr); // snowflake
      break;
    case 50:
    case 150:
      for(int i = 20; i>4; i-=4) {
        graphics_draw_line(ctx,GPoint(6,i+1),GPoint(10,i));
        graphics_draw_line(ctx,GPoint(10,i),GPoint(14,i+1));
        graphics_draw_line(ctx,GPoint(14,i+1),GPoint(18,i));
        graphics_draw_line(ctx,GPoint(18,i),GPoint(22,i+1));
      }
      break;
  }
}
