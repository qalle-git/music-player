#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#define NODE_ID 0

#include "TinyTimber.h"
#include "canTinyTimber.h"
#include <stdbool.h>

typedef enum {
  DISCONNECTED,
  CONDUCTOR,
  MUSICIAN,
} MUSIC_PLAYER_STATE;

// CAN Actions, in priority order.
typedef enum {
  STOP_MUSIC,
  PLAY_MUSIC,
  CHANGE_VOLUME,
  CHANGE_TEMPO,
  CHANGE_KEY,
  TOGGLE_MUTE,
  TOGGLE_IS_PLAYING,
} CAN_ACTION;

// CAN
bool can_action(CANMsg *msg);
bool send_can_action(Can *self, CAN_ACTION can_action, char *data,
                     int data_length);

#endif