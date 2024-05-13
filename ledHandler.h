#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "TinyTimber.h"
#include "melody.h"

typedef enum {
  LED_DISABLED = -1,
  LED_OFF,
  LED_ON,
} LED_STATE;

typedef struct {
  Object super;

  int blink_period;
  int current_blink_period;

  LED_STATE state;

  Msg led_tick_call;
} LedHandler;

#define initLedHandler()                                                       \
  {                                                                            \
    initObject(), 60000 / (DEFAULT_BPM / 2), 60000 / (DEFAULT_BPM / 2),        \
        LED_OFF, NULL                                                          \
  }

void set_led(LedHandler *self, LED_STATE state);
void set_led_blink_period(LedHandler *self, int period);
void set_next_tone(LedHandler *self, int unused);

void toggle_led(LedHandler *self, int unused);

void led_tick(LedHandler *self, int unused);

#endif