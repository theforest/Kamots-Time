/*
*** Kamots Time ***

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

int confver = 0;
appConfig conf;
uint8_t config_changed = 0;
Weather wx;
char text_wx_t[] = "---.-";
char text_wx_ft[] = "F";

// Phone communication keys
#define C_HH 1 // Color Hour Hand
#define C_MH 2 // Color Minute Hand
#define C_HM 3 // Color Hour Marks
#define C_WB 4 // Color Watchface Background
#define C_WO 5 // Color Watchface Outline (also includes text/icons in surround)
#define C_SB 6 // Color Surround Background
#define D_DT 7 // Display Digital time
#define HM_C 8 // Hour Marks Count
#define D_BT 9 // Display Bluetooth
#define C_SH 10 // Color Second Hand
#define D_SH 11 // Display Second Hand
#define DT_Z 12 // Digital Time is Zulu
#define D_WX 13 // Display Weather
#define WX_T 14 // Weather Temperature
#define WX_C 15 // Weather Conditions
#define WX_A 16 // Weather received At time
#define TOFF 17 // GMT Offset of local time
#define WFRQ 18 // Weather Update Frequency
#define WFMT 19 // Weather Temperature Format

// Persistant storage keys
const uint32_t KEY_CONFVER = 52668701; // int - configuration version ID
const uint32_t KEY_CONFDAT = 52668711; // data - configuration data

const uint8_t CURRENT_CONFVER = 7; // MUST CHANGE THIS if appConfig struct changes

appConfig load_defaults() { // fill the default configuration values
  appConfig defaultconf;
  defaultconf.color_hour_hand = COLOR_FALLBACK(GColorRed,GColorBlack);
  defaultconf.color_minute_hand = COLOR_FALLBACK(GColorBlue,GColorBlack);
  defaultconf.color_hour_markers = COLOR_FALLBACK(GColorDarkGreen,GColorBlack);
  defaultconf.color_watchface_background = GColorWhite;
  defaultconf.color_watchface_outline = GColorBlack;
  defaultconf.color_surround_background = COLOR_FALLBACK(GColorDarkGreen,GColorClear);
  defaultconf.display_digital = false; // Default not displaying digital time
  defaultconf.color_second_hand = GColorBlack;
  defaultconf.hour_markers_count = 12; // Show all hour markers by default
  defaultconf.display_bt_status = false; // Default not displaying bluetooth status
  defaultconf.display_second_hand = false;
  defaultconf.digital_as_zulu = false;
  defaultconf.display_weather = false;
  defaultconf.weather_update_frequency = 20; // minutes
  defaultconf.weather_temp_format = true; // true=F, false=C
  return defaultconf;
}

bool trigger_weather(void) {
  if(!bt_connected) return false; // If bluetooth isn't connected, don't bother trying
  AppMessageResult res;
  DictionaryIterator *iter;
  DictionaryResult dres;
  uint8_t wxflag = 1; // 1=GetWX (C), 2=GetWX (F)

  wxflag = wxflag + (unsigned int)conf.weather_temp_format; // Pass along temperature format

  app_message_outbox_begin(&iter);
  if(iter == NULL) return false;
  
  dres = dict_write_uint8(iter, 99, wxflag);
  dict_write_end(iter);

  if(dres != DICT_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Dict error %d", (unsigned int)dres);
    return false;
  }

  res = app_message_outbox_send();
  if(res == APP_MSG_OK) return true;
  else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox error %d", (unsigned int)res);
    return false;
  }
  return false; // Should never get here
}

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received %d bytes", (unsigned int)dict_size(iterator));
  int8_t hm_count = 0;
  int8_t wx_freq = 20;
  int32_t colorint = 0;
  bool wxupdate = false;

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
      config_changed++;
      break;
#if defined(PBL_COLOR)
    case C_HH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_hour_hand = GColorFromHEX(colorint);
      config_changed++;
      break;

    case C_MH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_minute_hand = GColorFromHEX(colorint);
      break;

    case C_SH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_second_hand = GColorFromHEX(colorint);
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
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_hour_hand = GColorWhite;
      if(colorint == 0x000000) conf.color_hour_hand = GColorBlack;
      config_changed++;
      break;

    case C_MH:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_minute_hand = GColorWhite;
      if(colorint == 0x000000) conf.color_minute_hand = GColorBlack;
      break;

    case C_SH:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_second_hand = GColorWhite;
      if(colorint == 0x000000) conf.color_second_hand = GColorBlack;
      break;

    case C_HM:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_hour_markers = GColorWhite;
      if(colorint == 0x000000) conf.color_hour_markers = GColorBlack;
      break;

    case C_WB:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_watchface_background = GColorWhite;
      if(colorint == 0x000000) conf.color_watchface_background = GColorBlack;
      break;

    case C_WO:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_watchface_outline = GColorWhite;
      if(colorint == 0x000000) conf.color_watchface_outline = GColorBlack;
      break;

    case C_SB:
      colorint = t->value->int32;
      if(colorint == 0xFFFFFF) conf.color_surround_background = GColorWhite;
      if(colorint == 0x000000) conf.color_surround_background = GColorBlack;
      break;
#endif
    case HM_C:
      hm_count = t->value->int8;
      if(hm_count == 12) conf.hour_markers_count = 12;
      if(hm_count == 4) conf.hour_markers_count = 4;
      if(hm_count == 1) conf.hour_markers_count = 1;
      config_changed++;
      break;

    case D_BT:
      if (t->value->int8 == 1) {
        conf.display_bt_status = true;
      } else {
        conf.display_bt_status = false;
      }
      config_changed++;
      break;

    case D_SH:
      if (t->value->int8 == 1) {
        conf.display_second_hand = true;
      } else {
        conf.display_second_hand = false;
      }
      config_changed++;
      break;

    case DT_Z:
      if (t->value->int8 == 1) {
        conf.digital_as_zulu = true;
      } else {
        conf.digital_as_zulu = false;
      }
      config_changed++;
      break;

    case D_WX:
      if (t->value->int8 == 1) {
        conf.display_weather = true;
      } else {
        conf.display_weather = false;
      }
      config_changed++;
      break;

    case WX_T:
      if(t->value->int16 < 2000) {
        wx.temperature = (float)t->value->int16 / 10.0;
      } else {
        wx.temperature = 222.0;
      }
      wxupdate = true;
      break;

    case WX_C:
      if(t->value->uint8 > 0) {
        wx.conditions = t->value->uint8;
      } else {
        wx.conditions = 0;
      }
      wxupdate = true;
      break;

    case WX_A:
      if(t->value->uint32 > 10000000) {
        wx.timestamp = t->value->uint32;
      } else {
        wx.timestamp = 1;
      }
      wxupdate = true;
      break;

    case TOFF:
        wx.gmtoffset = t->value->uint32;
      break;

    case WFRQ:
      wx_freq = t->value->int8;
      if(wx_freq == 20) conf.weather_update_frequency = 20;
      if(wx_freq == 30) conf.weather_update_frequency = 30;
      if(wx_freq == 40) conf.weather_update_frequency = 40;
      if(wx_freq == 45) conf.weather_update_frequency = 45;
      if(wx_freq == 50) conf.weather_update_frequency = 50;
      if(wx_freq == 60) conf.weather_update_frequency = 60;
      if(wx_freq == 120) conf.weather_update_frequency = 120;
      config_changed++;
      break;

    case WFMT:
      if (t->value->int8 == 1) {
        conf.weather_temp_format = true;
      } else {
        conf.weather_temp_format = false;
      }
      config_changed++;
      break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  if(config_changed > 1) { // configuration changed, we need to reinitialize
    config_changed = 0;
    if(conf.weather_temp_format) strncpy(text_wx_ft, "F", 1);
    else strncpy(text_wx_ft, "C", 1);
    reload(); // reload everything
  }
  if(wxupdate && conf.display_weather) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WX: t=%d, c=%u, a=%u", (int)wx.temperature, wx.conditions, (unsigned int)wx.timestamp);
    if(wx.conditions > 200 || wx.conditions == 0) {
      if(!app_timer_reschedule(atwx,300000)) atwx = app_timer_register(300000, handle_app_timer_weather, NULL); // Schedule retry
      strncpy(text_wx_t,"---.-",sizeof(text_wx_t));
    } else {
      if(!app_timer_reschedule(atwx,conf.weather_update_frequency * 60000)) atwx = app_timer_register(conf.weather_update_frequency * 60000, handle_app_timer_weather, NULL); // Schedule next update
      ftoa(text_wx_t,wx.temperature,1);
    }
    weather_calc_age();
    if(weather_t_layer) {
      layer_mark_dirty(text_layer_get_layer(weather_t_layer)); // Text layers are supposed to auto-update, but it is slow
    }
    if(weather_c_layer) {
      layer_mark_dirty(weather_c_layer);
    }
  }
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped! Err: %d", reason);
} 

void outbox_failed_callback(DictionaryIterator *iter ,AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox Message failed! Err: %d", reason);
  if(reason == APP_MSG_SEND_TIMEOUT) {
    if(!app_timer_reschedule(atwx,60000)) atwx = app_timer_register(60000, handle_app_timer_weather, NULL);
    wx.conditions = 204;
    strncpy(text_wx_t,"---.-",sizeof(text_wx_t));
  }
}

void convertconfig() {
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
        // no break until last case, continue with conversion

      case 2:
        newconf.hour_markers_count = defaultconf.hour_markers_count;
        newconf.display_bt_status = defaultconf.display_bt_status;

      case 3:
        newconf.color_second_hand = defaultconf.color_second_hand;
        newconf.display_second_hand = defaultconf.display_second_hand;
        newconf.digital_as_zulu = defaultconf.digital_as_zulu;

      case 4:
        newconf.display_weather = defaultconf.display_weather;

      case 5:
        newconf.weather_update_frequency = defaultconf.weather_update_frequency;
        newconf.weather_temp_format = defaultconf.weather_temp_format;

      case 6:
        if(((int)conf.color_hour_hand.argb == 0) && !gcolor_equal(conf.color_hour_hand,GColorBlack)) newconf.color_hour_hand = GColorBlack;
        if(((int)conf.color_hour_hand.argb == 1) && !gcolor_equal(conf.color_hour_hand,GColorWhite)) newconf.color_hour_hand = GColorWhite;
        if(((int)conf.color_minute_hand.argb == 0) && !gcolor_equal(conf.color_minute_hand,GColorBlack)) newconf.color_minute_hand = GColorBlack;
        if(((int)conf.color_minute_hand.argb == 1) && !gcolor_equal(conf.color_minute_hand,GColorWhite)) newconf.color_minute_hand = GColorWhite;
        if(((int)conf.color_hour_markers.argb == 0) && !gcolor_equal(conf.color_hour_markers,GColorBlack)) newconf.color_hour_markers = GColorBlack;
        if(((int)conf.color_hour_markers.argb == 1) && !gcolor_equal(conf.color_hour_markers,GColorWhite)) newconf.color_hour_markers = GColorWhite;
        if(((int)conf.color_watchface_background.argb == 0) && !gcolor_equal(conf.color_watchface_background,GColorBlack)) newconf.color_watchface_background = GColorBlack;
        if(((int)conf.color_watchface_background.argb == 1) && !gcolor_equal(conf.color_watchface_background,GColorWhite)) newconf.color_watchface_background = GColorWhite;
        if(((int)conf.color_watchface_outline.argb == 0) && !gcolor_equal(conf.color_watchface_outline,GColorBlack)) newconf.color_watchface_outline = GColorBlack;
        if(((int)conf.color_watchface_outline.argb == 1) && !gcolor_equal(conf.color_watchface_outline,GColorWhite)) newconf.color_watchface_outline = GColorWhite;
        if(((int)conf.color_surround_background.argb == 0) && !gcolor_equal(conf.color_surround_background,GColorBlack)) newconf.color_surround_background = GColorBlack;
        if(((int)conf.color_surround_background.argb == 1) && !gcolor_equal(conf.color_surround_background,GColorWhite)) newconf.color_surround_background = GColorWhite;
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
