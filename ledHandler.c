#include "ledHandler.h"
#include "application.h"

void set_led(LedHandler *self, LED_STATE state) {
  self->state = state;

  if (state == LED_ON) {
    SIO_WRITE(&sio, 1);
  } else if (state == LED_OFF) {
    SIO_WRITE(&sio, 0);
  }
}

void toggle_led(LedHandler *self, int unused) {
  if (self->state == LED_ON) {
    set_led(self, LED_OFF);
  } else {
    set_led(self, LED_ON);
  }
}

void set_led_blink_period(LedHandler *self, int period) {
  self->blink_period = 60000 / period / 2;

  // ABORT(self->led_tick_call);

  // self->led_tick_call =
  //     SEND(MSEC(self->blink_period), USEC(50), self, led_tick, 0);
}

void set_next_tone(LedHandler *self, int unused) {
  if (self->current_blink_period == self->blink_period)
    return;

  self->current_blink_period = self->blink_period;

  ABORT(self->led_tick_call);

  self->led_tick_call =
      SEND(MSEC(self->current_blink_period), USEC(100), self, led_tick, 0);
}

void led_tick(LedHandler *self, int unused) {
  self->led_tick_call = NULL;

  if (self->state == LED_DISABLED) {
    SIO_WRITE(&sio, 1);

    return;
  }

  if (self->state == LED_ON) {
    set_led(self, LED_OFF);
  } else {
    set_led(self, LED_ON);
  }

  self->led_tick_call =
      SEND(MSEC(self->current_blink_period), USEC(100), self, led_tick, 0);
}