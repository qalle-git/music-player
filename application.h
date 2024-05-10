#ifndef APPLICATION_H
#define APPLICATION_H

#include "canHandler.h"
#include "ledHandler.h"
#include "sioTinyTimber.h"

#define initApp()                                                              \
  { initObject(), 0, {0}, DISCONNECTED }

typedef struct {
  Object super;
  int index;
  char buffer[128];

  MUSIC_PLAYER_STATE state;
} App;

void print(char *format, int arg);
void print_raw(char *string);

MUSIC_PLAYER_STATE get_state(App *self);

extern App app;
extern SysIO sio;
extern LedHandler led_handler;

#endif