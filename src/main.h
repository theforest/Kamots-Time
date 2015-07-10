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

#endif