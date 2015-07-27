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
  
int confver = 0;
appConfig conf;
uint8_t config_changed = 0;

// Phone communication keys
#define C_HH 1
#define C_MH 2
#define C_HM 3
#define C_WB 4
#define C_WO 5
#define C_SB 6
#define D_DT 7
#define HM_C 8
#define D_BT 9

// Persistant storage keys
const uint32_t KEY_CONFVER = 52668701; // int - configuration version ID
const uint32_t KEY_CONFDAT = 52668711; // data - configuration data

const uint8_t CURRENT_CONFVER = 3; // MUST CHANGE THIS if appConfig struct changes

appConfig load_defaults() { // fill the default configuration values
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

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received %d bytes", (unsigned int)dict_size(iterator));
  int8_t hm_count = 0;
  int32_t colorint = 0;

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
#ifdef PBL_COLOR
    case C_HH:
      colorint = t->value->int32;
      if(colorint >= 0x0 && colorint <= 0xFFFFFF) conf.color_hour_hand = GColorFromHEX(colorint);
      config_changed++;
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

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  if(config_changed > 1) { // configuration changed, we need to reinitialize
    config_changed = 0;
    reload(); // reload everything
  }
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox Message dropped! Err: %d", reason);
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
