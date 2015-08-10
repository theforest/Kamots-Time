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

Time anim_time;
bool animating = false;
int radius = 0;
#define FINAL_RADIUS 68

static void animation_started(Animation *anim, void *context) {
  animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  animating = false;
}

void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(clock_layer);
}

void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(last_time.hours));
  anim_time.minutes = anim_percentage(dist_normalized, last_time.minutes);
  if(conf.display_second_hand) anim_time.seconds = anim_percentage(dist_normalized, last_time.seconds);

  layer_mark_dirty(clock_layer);
}
