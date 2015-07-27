#pragma once
#include <pebble.h>
extern int map(int x, int in_min, int in_max, int out_min, int out_max);
extern int hours_to_minutes(int hours_out_of_12);
extern void graphics_draw_line2(GContext *ctx, GPoint p0, GPoint p1, int8_t width);
#if !PBL_COLOR
extern void graphics_context_set_stroke_width(struct GContext *ctx, int width);
#endif