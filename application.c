/*
 * User's Guide:
 *
 * This program is a real-time system that generates tones and applies a
 * distortion effect. It uses the TinyTimber framework for event-driven
 * programming.
 *
 * Usage:
 * Use the following commands to interact with the program:
 *  - 'w': Increase the volume of the tone.
 *  - 's': Decrease the volume of the tone.
 *  - 'm': Toggle mute on/off for the tone.
 *  Write numbers and press 't': Enter a new tempo (beats per minute).
 *  Write numbers and press 'k': Enter a new key offset.
 *
 * Note: The program uses a DAC (Digital-to-Analog Converter) to generate the
 * tone output. Make sure the DAC is properly connected to the device running
 * the program.
 *
 * Author: Carl, Kevin, Carl
 * Date: 2024-04-22
 */

#include "TinyTimber.h"
#include "canTinyTimber.h"
#include "ledHandler.h"
#include "sciTinyTimber.h"
#include <complex.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "application.h"
#include "buttonHandler.h"
#include "canHandler.h"
#include "musicPlayer.h"
#include "sioTinyTimber.h"
#include "toneGenerator.h"

void start_app(App *self, int unused);

void reader(App *self, int);
void receiver(App *self, int);

App app = initApp();
MusicPlayer music_player = initMusicPlayer();
ToneGenerator tone_generator = initToneGenerator();

ButtonHandler button_handler = initButtonHandler();
LedHandler led_handler = initLedHandler();

Serial sci0 = initSerial(SCI_PORT0, &app, reader);
Can can0 = initCan(CAN_PORT0, &app, receiver);

SysIO sio = initSysIO(SIO_PORT0, &button_handler, sio_reader);

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  INSTALL(&sio, sio_interrupt, SIO_IRQ0);

  TINYTIMBER(&app, start_app, 0);

  return 0;
}

void print(char *format, int arg) {
  char buffer[128];
  snprintf(buffer, 128, format, arg);
  SCI_WRITE(&sci0, buffer);
}

void print_raw(char *string) {
  char buffer[128];
  snprintf(buffer, 128, "%s", string);
  SCI_WRITE(&sci0, buffer);
}

void start_app(App *self, int unused) {
  CAN_INIT(&can0);
  SCI_INIT(&sci0);
  SIO_INIT(&sio);

  print_raw("Welcome to the Music Player!\n");
  print_raw("/-----------------------------------\\\n");
  print_raw("Press 'e' to connect as conductor.\n");
  print_raw("Press 'd' to connect as musician.\n");
  print_raw("\n");
  print_raw("Press 'w' to increase volume.\n");
  print_raw("Press 's' to decrease volume.\n");
  print_raw("Press 'm' to toggle mute.\n");
  print_raw("Press 't' to enter tempo.\n");
  print_raw("Press 'k' to enter key.\n");
  print_raw("Press 'v' to play music.\n");
  print_raw("Press 'x' to stop music.\n");
}

void receiver(App *self, int unused) {
  if (self->state == DISCONNECTED)
    return;

  CANMsg msg;

  CAN_RECEIVE(&can0, &msg);

  can_action(&msg, self->state);
}

/**
 * Handles the input from the reader and performs corresponding actions.
 *
 * @param self Pointer to the App structure.
 * @param c The input character.
 */
void reader(App *self, int c) {
  if (c == '\n')
    return;

  switch (c) {
  case 'w':
    send_can_action(&can0, CHANGE_VOLUME, "1", 1);

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, change_music_volume, 1))
        print("Volume: %d\n", tone_generator.volume);
      else
        print_raw("Volume is already maximum.\n");
    }

    break;
  case 's':
    send_can_action(&can0, CHANGE_VOLUME, "-1", 2);

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, change_music_volume, -1))
        print("Volume: %d\n", tone_generator.volume);
      else
        print_raw("Volume is already minimum.\n");
    }

    break;
  case 'm':
    send_can_action(&can0, TOGGLE_MUTE, "0", 1);

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, toggle_music_mute, 0))
        print_raw("Music is muted.\n");
      else
        print_raw("Music is not muted.\n");
    }

    break;
  case 't':
    self->buffer[self->index] = '\0'; // Terminate string

    send_can_action(&can0, CHANGE_TEMPO, self->buffer, self->index);

    int bpm = atoi(self->buffer); // Convert string to int

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, change_tempo, bpm))
        print("Changed Tempo: %d\n", bpm);
      else
        print_raw("Tempo out of range\n");
    }

    self->index = 0;

    break;
  case 'k':
    self->buffer[self->index] = '\0'; // Terminate string

    send_can_action(&can0, CHANGE_KEY, self->buffer, self->index);

    int new_key = atoi(self->buffer); // Convert string to int

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, change_key, new_key))
        print("Changed Key: %d\n", new_key);
      else
        print_raw("Key out of range\n");
    }

    self->index = 0;

    break;
  case 'v':
    send_can_action(&can0, PLAY_MUSIC, "0", 1);

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, start_music, 0))
        print_raw("Music is now playing from the beginning.\n");
      else
        print_raw("Music is already playing.\n");
    }

    break;
  case 'x':
    send_can_action(&can0, STOP_MUSIC, "0", 1);

    if (self->state == CONDUCTOR) {
      if (SYNC(&music_player, stop_music, 0))
        print_raw("Music is now stopped.\n");
      else
        print_raw("Music is already stopped.\n");
    }

    break;
  case 'e':
    self->state = CONDUCTOR;

    print_raw("Connected as conductor!\n");

    break;
  case 'd':
    self->state = MUSICIAN;

    print_raw("Connected as musician!\n");

    break;
  default:
    if (!isdigit(c) && c != '-')
      return;

    print("Entered: %c\n", c);

    self->buffer[self->index++] = c; // store integer in buffer
  }
}

MUSIC_PLAYER_STATE get_state(App *self) { return self->state; }