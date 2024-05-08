#ifndef TONE_GENERATOR_H
#define TONE_GENERATOR_H

#include "TinyTimber.h"
#include <stdbool.h>

typedef struct {
  Object super;

  bool is_not_in_gap;
  bool is_muted;

  int frequency;
  int period;

  int wave_flip;

  int volume;
} ToneGenerator;

void stop_tone(ToneGenerator *self);

void tone_tick(ToneGenerator *self);

bool toggle_mute(ToneGenerator *self);
bool toggle_is_playing(ToneGenerator *self);

int change_volume(ToneGenerator *self, int increment);

bool set_frequency(ToneGenerator *self, int frequency);

extern ToneGenerator tone_generator;

#endif