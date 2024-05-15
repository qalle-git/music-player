#include "canHandler.h"
#include "application.h"
#include "canHandler.h"
#include "canTinyTimber.h"
#include "musicPlayer.h"
#include <stdbool.h>
#include <stdlib.h>

bool can_action(CANMsg *msg, MUSIC_PLAYER_STATE user_state) {
  const int action_id = msg->msgId;

  const int data = atoi((char *)msg->buff);
  const int data_length = msg->length;

  print("Can Received: Action: %d ", action_id);
  print("Data: %d\n", data);

  if (user_state == DISCONNECTED || user_state == CONDUCTOR) {
    print_raw(
        "User is ignoring can message, either disconnected or a conductor.\n");

    return false;
  }

  switch (action_id) {
  case PLAY_MUSIC:
    if (SYNC(&music_player, start_music, 0)) {
      print_raw("Music is now playing from the beginning.\n");
    } else {
      print_raw("Music is already playing.\n");
    }

    break;
  case STOP_MUSIC:
    if (SYNC(&music_player, stop_music, 0)) {
      print_raw("Music is now stopped.\n");
    } else {
      print_raw("Music is already stopped.\n");
    }

    break;
  case CHANGE_VOLUME:
    if (data_length > 0) {
      int new_volume = SYNC(&music_player, change_music_volume, data);

      if (new_volume) {
        print("Volume: %d\n", new_volume);
      } else {
        if (data > 0) {
          print_raw("Volume is already maximum.\n");
        } else {
          print_raw("Volume is already minimum.\n");
        }
      }

      return new_volume;
    }

    break;
  case CHANGE_TEMPO:
    if (data_length > 0) {
      if (SYNC(&music_player, change_tempo, data)) {
        print("Changed Tempo: %d\n", data);
      } else {
        print_raw("Tempo out of range\n");
      }

      return true;
    }

    break;
  case CHANGE_KEY:
    if (data_length > 0) {
      if (SYNC(&music_player, change_key, data)) {
        print("Changed Key: %d\n", data);
      } else {
        print_raw("Key out of range\n");
      }

      return true;
    }

    break;
  case TOGGLE_MUTE:
    if (SYNC(&music_player, toggle_music_mute, 0)) {
      print_raw("Music is muted.\n");
    } else {
      print_raw("Music is not muted.\n");
    }

    return true;
  case TOGGLE_IS_PLAYING:
    break;
  default:
    break;
  }

  return false;
}

bool send_can_action(Can *self, CAN_ACTION can_action, char *data,
                     int data_length) {
  if (data_length > CAN_BUFSIZE) {
    return false;
  }

  CANMsg can_msg;

  can_msg.msgId = can_action;
  can_msg.nodeId = NODE_ID;

  for (int i = 0; i < data_length; i++) {
    can_msg.buff[i] = data[i];
  }

  can_msg.length = data_length;

  CAN_SEND(self, &can_msg);

  return true;
}