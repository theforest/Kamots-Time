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
    lastwxage = time(NULL)-29;
    text_wx_age[0] = 0;
  }
  if(lastwxage <= (time(NULL)-30)) {
    #ifdef PBL_PLATFORM_APLITE
    age = (time(NULL) - (wx.timestamp-wx.gmtoffset)) / 60;
    #else
    age = (time(NULL) - wx.timestamp) / 60;
    #endif
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

#if !PBL_COLOR
void graphics_context_set_stroke_width(struct GContext *ctx, uint8_t width) {
  stroke_width = width;
}
// Draw line with width. From: http://forums.getpebble.com/discussion/comment/132110/#Comment_132110
// (Based on code found here http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C)
void graphics_draw_line2(GContext *ctx, GPoint p0, GPoint p1) {
  uint8_t width = stroke_width;
  // Order points so that lower x is first
  int16_t x0, x1, y0, y1;
  if (p0.x <= p1.x) {
    x0 = p0.x; x1 = p1.x; y0 = p0.y; y1 = p1.y;
  } else {
    x0 = p1.x; x1 = p0.x; y0 = p1.y; y1 = p0.y;
  }
    
  // Init loop variables
  int16_t dx = x1-x0;
  int16_t dy = abs(y1-y0);
  int16_t sy = y0<y1 ? 1 : -1; 
  int16_t err = (dx>dy ? dx : -dy)/2;
  int16_t e2;
  
  // Calculate whether line thickness will be added vertically or horizontally based on line angle
  int8_t xdiff, ydiff;
  
  if (dx > dy) {
    xdiff = 0;
    ydiff = width/2;
  } else {
    xdiff = width/2;
    ydiff = 0;
  }
    
  // Use Bresenham's integer algorithm, with slight modification for line width, to draw line at any angle
  while (true) {
    // Draw line thickness at each point by drawing another line 
    // (horizontally when > +/-45 degrees, vertically when <= +/-45 degrees)
    graphics_draw_line(ctx, GPoint(x0-xdiff, y0-ydiff), GPoint(x0+xdiff, y0+ydiff));
    
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0++; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}
#endif
