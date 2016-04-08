#include "util.h"
#include "main.h"

uint8_t stroke_width = 1;
time_t lastwxage = 0;

int map(int x, int in_min, int in_max, int out_min, int out_max) { // Borrowed from Arduino
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

void weather_calc_age() {
  unsigned int age = 0;
  if(wx.conditions == 0) {
    lastwxage = time(NULL)-28;
    text_wx_age[0] = 0;
    if(weather_a_layer) {
      layer_mark_dirty(text_layer_get_layer(weather_a_layer)); // Text layers are supposed to auto-update, but it is slow
    }
  } else if(wx.conditions > 199) {
    if(wx.conditions == 201) {
      strncpy(text_wx_age,"GPS?",sizeof(text_wx_age));
    } else if(wx.conditions == 202) {
      strncpy(text_wx_age,"NET?",sizeof(text_wx_age));
    } else if(wx.conditions == 203) {
      strncpy(text_wx_age,"API?",sizeof(text_wx_age));
    } else if(wx.conditions == 204) {
      strncpy(text_wx_age,"PHN?",sizeof(text_wx_age));
    } else {
      strncpy(text_wx_age,"ERR!",sizeof(text_wx_age));
    }
    if(weather_a_layer) {
      layer_mark_dirty(text_layer_get_layer(weather_a_layer)); // Text layers are supposed to auto-update, but it is slow
    }
  }

  if(lastwxage <= (time(NULL)-30)) {
    age = (time(NULL) - wx.timestamp) / 60;
    if(age < 100) snprintf(text_wx_age, sizeof(text_wx_age), "%um", age);
    else strncpy(text_wx_age, "99m+", sizeof(text_wx_age));
    lastwxage = time(NULL);
    if(weather_a_layer) {
      layer_mark_dirty(text_layer_get_layer(weather_a_layer)); // Text layers are supposed to auto-update, but it is slow
    }
  }
}

void ftoa(char* str, double val, int precision) { // From http://forums.getpebble.com/discussion/comment/73084/#Comment_73084
  //  start with positive/negative
  if (val < 0) {
    *(str++) = '-';
    val = -val;
  }
  //  integer value
  snprintf(str, 12, "%d", (int) val);
  str += strlen(str);
  val -= (int) val;
  //  decimals
  if ((precision > 0) && (val >= .00001)) {
    //  add period
    *(str++) = '.';
    //  loop through precision
    for (int i = 0;  i < precision;  i++)
      if (val > 0) {
        val *= 10;
        *(str++) = '0' + (int) (val + ((i == precision - 1) ? .5 : 0));
        val -= (int) val;
      } else
        break;
  }
  //  terminate
  *str = '\0';
}

