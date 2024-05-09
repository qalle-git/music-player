#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "TinyTimber.h"

typedef enum {
  BUTTON_PRESSED,
  BUTTON_RELEASED,
} BUTTON_STATE;

typedef enum { BUTTON_PRESS, BUTTON_HOLD } BUTTON_MODE;

#define initButtonHandler()                                                    \
  { initObject(), BUTTON_PRESS, {}, 0, NULL }

#define CONTACT_BOUNCE_FILTER_MS 100

#define MAX_BURST_LENGTH 3
#define BURST_COMPATIBLE_MS 100

typedef struct {
  Object super;

  BUTTON_MODE mode;

  int burst[MAX_BURST_LENGTH];
  int burst_length;

  Msg hold_call;
} ButtonHandler;

Time get_time_since_last_button_press();

void sio_reader(ButtonHandler *self, int unused);

void button_was_held(ButtonHandler *self, int unused);
void button_was_released(ButtonHandler *self, int milliseconds_since_press);

int calculate_burst_average(ButtonHandler *self);

#endif