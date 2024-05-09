#include "TinyTimber.h"
#include "sioTinyTimber.h"

#include "application.h"
#include "buttonHandler.h"
#include <stdlib.h>

Timer last_button_press = initTimer();
Timer held_timer = initTimer();

Time get_time_since_last_button_press() { return T_SAMPLE(&last_button_press); }

void sio_reader(ButtonHandler *self, int unused) {
  int button_state = SIO_READ(&sio);

  const int time_since_last_press = MSEC_OF(get_time_since_last_button_press());

  if (time_since_last_press < CONTACT_BOUNCE_FILTER_MS) {
    return;
  }

  if (button_state == BUTTON_PRESSED) {
    self->mode = BUTTON_PRESS;

    SIO_TRIG(&sio, BUTTON_RELEASED);

    print_raw("Button pressed\n");

    self->hold_call = AFTER(SEC(1), self, button_was_held, 0);

    T_RESET(&last_button_press);
  } else {
    SIO_TRIG(&sio, BUTTON_PRESSED);

    ABORT(self->hold_call);

    if (self->mode == BUTTON_HOLD) {
      print("Button was held for %d seconds after initiating press-and-hold "
            "mode.\n",
            SEC_OF(T_SAMPLE((&held_timer))));
    } else {
      button_was_released(self, time_since_last_press);
    }

    T_RESET(&last_button_press);
  }
}

void button_was_held(ButtonHandler *self, int unused) {
  self->mode = BUTTON_HOLD;

  print_raw("Button held for 1 second.\n");

  T_RESET(&held_timer);
}

void button_was_released(ButtonHandler *self, int milliseconds_since_press) {
  print("Button released after %d milliseconds.\n", milliseconds_since_press);

  if (self->burst_length > 0) {
    for (int burst_index = 0; burst_index < self->burst_length; burst_index++) {
      const int burst = self->burst[burst_index];

      if (abs(burst - milliseconds_since_press) > BURST_COMPATIBLE_MS) {
        print("Burst is not compatible with %dms.\n", milliseconds_since_press);

        self->burst_length = 0;

        return;
      }
    }
  }

  self->burst[self->burst_length++] = milliseconds_since_press;

  print("Burst added to buffer: %d\n", milliseconds_since_press);

  if (self->burst_length == MAX_BURST_LENGTH) {
    print_raw("Burst buffer is full.\n");

    const int burst_average = calculate_burst_average(self);

    const int burst_bpm = 60000 / burst_average;

    print("Set Tempo to %d BPM\n", burst_bpm);

    self->burst_length = 0;

    return;
  }
}

int calculate_burst_average(ButtonHandler *self) {
  int sum = 0;

  for (int burst_index = 0; burst_index < self->burst_length; burst_index++) {
    sum += self->burst[burst_index];
  }

  const int average = sum / self->burst_length;

  return average;
}