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

void set_led_blink_period(LedHandler *self, int period) {
  self->blink_period = 60000 / period;
}

void led_tick(LedHandler *self, int unused) {
  if (self->state == LED_DISABLED) {
    set_led(self, LED_ON);

    return;
  }

  if (self->state == LED_ON) {
    set_led(self, LED_OFF);
  } else {
    set_led(self, LED_ON);
  }

  SEND(MSEC(self->blink_period), USEC(50), self, led_tick, 0);
}