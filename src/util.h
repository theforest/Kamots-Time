#pragma once
#include <pebble.h>
extern int map(int x, int in_min, int in_max, int out_min, int out_max);
extern int hours_to_minutes(int hours_out_of_12);
extern void ftoa(char* str, double val, int precision);
extern void weather_calc_age();
