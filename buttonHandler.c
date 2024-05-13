#include "TinyTimber.h"
#include "canHandler.h"
#include "melody.h"
#include "musicPlayer.h"
#include "sioTinyTimber.h"

#include "application.h"
#include "buttonHandler.h"
#include <stdlib.h>

Timer last_button_press = initTimer();
Timer last_release_press = initTimer();
Timer held_timer = initTimer();

Time get_time_since_last_button_press() { return T_SAMPLE(&last_button_press); }

void sio_reader(ButtonHandler *self, int unused) {
  int button_state = SIO_READ(&sio);

  const int time_since_last_press = get_time_since_last_button_press() / 100;

  if (button_state == BUTTON_PRESSED) {
    self->mode = BUTTON_PRESS;

    SIO_TRIG(&sio, BUTTON_RELEASED);

    if (time_since_last_press < CONTACT_BOUNCE_FILTER_MS) {
      return;
    }

    print_raw("Button pressed\n");

    self->hold_call = AFTER(SEC(1), self, button_was_held, 0);

    T_RESET(&last_button_press);
  } else {
    SIO_TRIG(&sio, BUTTON_PRESSED);

    ABORT(self->hold_call);

    if (time_since_last_press < CONTACT_BOUNCE_FILTER_MS) {
      return;
    }

    if (self->mode == BUTTON_HOLD) {
      const int held_for = SEC_OF(T_SAMPLE((&held_timer)));

      print("Button was held for %d seconds after initiating press-and-hold "
            "mode.\n",
            held_for);

      if (held_for >= 2) {
        bool changed =
            SYNC(&music_player, change_tempo_uncensored, DEFAULT_BPM);

        if (changed) {
          print("Tempo changed to %d BPM (Default).\n", DEFAULT_BPM);
        }
      } else {
        reset_burst(self);
      }
    } else {
      button_was_released(self);
    }

    T_RESET(&last_button_press);
  }
}

void button_was_held(ButtonHandler *self, int unused) {
  self->mode = BUTTON_HOLD;

  print_raw("Button held for 1 second.\n");

  T_RESET(&held_timer);
}

void button_was_released(ButtonHandler *self) {
  if (!self->first_release) {
    self->first_release = 1;

    T_RESET(&last_release_press);

    print_raw("Initiated tempo burst mode.\n");

    return;
  }

  int new_burst = (T_SAMPLE(&last_release_press)) / 100;

  if (self->burst_length > 0) {
    const int first_burst = self->burst[0];

    if (abs(first_burst - new_burst) > BURST_COMPATIBLE_MS) {
      print("Burst is not compatible with %dms gap.\n",
            abs(first_burst - new_burst));

      reset_burst(self);

      return;
    }
  }

  self->burst[self->burst_length++] = new_burst;

  print("Interval added to buffer: %d\n", new_burst);

  if (self->burst_length == MAX_BURST_LENGTH) {
    print_raw("Burst buffer is full.\n");

    const int burst_average = calculate_burst_average(self);

    const int burst_bpm = 60000 / burst_average;

    print("Set Tempo to %d BPM\n", burst_bpm);

    bool changed = SYNC(&music_player, change_tempo_uncensored, burst_bpm);

    if (changed) {
      print("Tempo changed to %d BPM.\n", burst_bpm);
    } else {
      print_raw("Tempo out of range, can't be set.\n");
    }

    reset_burst(self);

    return;
  }

  T_RESET(&last_release_press);
}

void reset_burst(ButtonHandler *self) {
  self->burst_length = 0;
  self->first_release = 0;

  T_RESET(&last_release_press);
}

int calculate_burst_average(ButtonHandler *self) {
  int sum = 0;

  for (int burst_index = 0; burst_index < self->burst_length; burst_index++) {
    sum += self->burst[burst_index];
  }

  const int average = sum / self->burst_length;

  return average;
}