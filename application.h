#ifndef APPLICATION_H
#define APPLICATION_H

#include "canHandler.h"

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

#endif