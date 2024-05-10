#include "musicPlayer.h"
#include "application.h"
#include "ledHandler.h"
#include "melody.h"
#include "toneGenerator.h"

bool start_music(MusicPlayer *self) {
  if (self->is_playing)
    return false;

  self->is_playing = true;
  self->tone_index = 0;

  ASYNC(self, player_tick, 0);

  SYNC(&led_handler, set_led, LED_ON);
  ASYNC(&led_handler, led_tick, 0);

  return true;
}

bool stop_music(MusicPlayer *self) {
  if (!self->is_playing)
    return false;

  self->is_playing = false;
  self->tone_index = 0;

  *DAC_ADDRESS = 0;

  SYNC(&tone_generator, stop_tone, 0);
  SYNC(&led_handler, set_led, LED_DISABLED);

  return true;
}

int change_music_volume(MusicPlayer *self, int increment) {
  int new_volume = SYNC(&tone_generator, change_volume, increment);

  return new_volume;
}

bool toggle_music_mute(MusicPlayer *self) {
  bool muted = SYNC(&tone_generator, toggle_mute, 0);

  return muted;
}

bool change_tempo(MusicPlayer *self, int bpm) {
  if (bpm < MIN_TEMPO || bpm > MAX_TEMPO)
    return false;

  self->tempo = bpm;

  ASYNC(&led_handler, set_led_blink_period, bpm / 2);

  return true;
}

bool change_tempo_uncensored(MusicPlayer *self, int bpm) {
  if (bpm < MIN_TEMPO - 30 || bpm > MAX_TEMPO + 60)
    return false;

  self->tempo = bpm;

  ASYNC(&led_handler, set_led_blink_period, bpm / 2);

  return true;
}

bool change_key(MusicPlayer *self, int key) {
  if (key > MAX_KEY || key < MIN_KEY)
    return false;

  self->key = key;

  return true;
}

void player_tick(MusicPlayer *self, int unused) {
  if (!self->is_playing)
    return;

  self->is_not_in_gap = SYNC(&tone_generator, toggle_is_playing, 0);

  if (self->is_not_in_gap) {
#ifdef TWINKLE
    int current_frequency =
        TWINKLE_MELODY_FREQUENCY_INDICES[self->tone_index] + self->key;
#elif defined(PIRATES)
    int current_frequency =
        PIRATES_MELODY_FREQUENCY_INDICES[self->tone_index] + self->key;
#elif defined(BROTHER_JOHN)
    int current_frequency =
        MELODY_FREQUENCY_INDICES[self->tone_index] + self->key;
#endif

    SYNC(&tone_generator, set_frequency, current_frequency);

#ifdef TWINKLE
    // Calculate the beat duration in milliseconds.
    int beatMS = 60000 / (self->tempo * TWINKLE_MELODY_BEATS[self->tone_index]);
#elif defined(PIRATES)
    // Calculate the beat duration in milliseconds.
    int beatMS = 60000 / (self->tempo * PIRATES_MELODY_BEATS[self->tone_index]);
#elif defined(BROTHER_JOHN)
    // Calculate the beat duration in milliseconds.
    int beatMS = 60000 / (self->tempo * MELODY_BEATS[self->tone_index]);
#endif

    ASYNC(&tone_generator, tone_tick, 0);

    // Go to next tone after the current beat.
    self->tone_index = (self->tone_index + 1) % 32;

    SEND(MSEC(beatMS - GAP_SILENCE), MSEC(beatMS), self, player_tick, 0);
  } else {
    SEND(MSEC(GAP_SILENCE), MSEC(GAP_SILENCE), self, player_tick, 0);
  }
}