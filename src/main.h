#ifndef MAIN_H
#define MAIN_H

// Global Stuff
#define COLORS       true
#define ANTIALIASING true

#define HAND_MARGIN  14
#define FINAL_RADIUS 68

#define ANIMATION_DURATION 400
#define ANIMATION_DELAY    500

const uint32_t KEY_CONFVER = 52668701; // int - configuration version
const uint32_t KEY_CONFDAT = 52668711; // data - configuration data

// Phone communication keys
#define KEY_COLOR_HH 1
#define KEY_COLOR_MH 2
#define KEY_COLOR_WO 3
#define KEY_COLOR_WB 4
#define KEY_COLOR_SB 5
#define KEY_DIGITIME 6

typedef struct {
  int hours;
  int minutes;
} Time;

typedef struct {
  GColor color_hour_hand;
  GColor color_minute_hand;
  GColor color_hour_markers;
  GColor color_watchface_outline;
  GColor color_watchface_background;
  GColor color_surround_background;
  bool display_digital;
// } __attribute__((__packed__)) appConfig;  // if needed as config grows
} appConfig;

appConfig conf;

#if !PBL_COLOR
void graphics_context_set_stroke_width(struct GContext *ctx, int width) {
  ;
}
#endif
#endif