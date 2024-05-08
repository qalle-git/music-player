#include "toneGenerator.h"
#include "melody.h"

void stop_tone(ToneGenerator *self) { self->is_not_in_gap = false; }

/**
 * Toggles the mute state of the application.
 *
 * @param self A pointer to the App structure.
 */
bool toggle_mute(ToneGenerator *self) {
  self->is_muted = !self->is_muted;

  if (self->is_muted)
    *DAC_ADDRESS = 0;

  return self->is_muted;
}

/**
 * Changes the volume of the application.
 *
 * @param self A pointer to the App structure representing the application.
 * @param increment The amount by which to increment the volume.
 */
int change_volume(ToneGenerator *self, int increment) {
  // Unmute if user tries to change the volume.
  if (self->is_muted)
    self->is_muted = false;

  if (self->volume + increment > MAX_VOLUME ||
      self->volume + increment < MIN_VOLUME)
    return false;

  self->volume += increment;

  return self->volume;
}

bool set_frequency(ToneGenerator *self, int frequency) {
  self->frequency = frequency;

  return true;
}

bool toggle_is_playing(ToneGenerator *self) {
  self->is_not_in_gap = !self->is_not_in_gap;

  return self->is_not_in_gap;
}

/**
 * Updates the tone output based on the given frequency period.
 * If the tone is not muted, it toggles the wave flip and sets the DAC address
 * accordingly. If the tone is muted, it sets the DAC address to 0. If the
 * deadline control is enabled, it schedules the next tone_tick event using
 * SEND. If the deadline control is disabled, it schedules the next tone_tick
 * event using AFTER.
 *
 * @param self The pointer to the App structure.
 * @param frequency_period The period of the frequency in microseconds.
 */
void tone_tick(ToneGenerator *self) {
  bool is_muted = self->is_muted;

  // Tone is muted, do not play. Go to next tick.
  if (!is_muted) {
    self->wave_flip ^= 1;

    int volume = self->volume;

    *DAC_ADDRESS = self->wave_flip * volume;
  } else
    *DAC_ADDRESS = 0;

  int frequency_period = get_period_from_frequency_indice(self->frequency);

  if (self->is_not_in_gap)
    SEND(USEC(frequency_period), USEC(100), self, tone_tick, 0);
}