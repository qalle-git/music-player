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
#include "sciTinyTimber.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_VOLUME 1
#define MAX_VOLUME 25

#define MIN_FREQUENCY_INDICE -10
#define MAX_FREQUENCY_INDICE 14

#define MIN_KEY -5
#define MAX_KEY 5

#define MIN_TEMPO 60
#define MAX_TEMPO 240

#define GAP_SILENCE 85

#define A 1.
#define B A / 2.
#define C A * 2.

const int MELODY_FREQUENCY_INDICES[32] = {0, 2, 4, 0, 0, 2,  4, 0, 4,  5, 7,
                                          4, 5, 7, 7, 9, 7,  5, 4, 0,  7, 9,
                                          7, 5, 4, 0, 0, -5, 0, 0, -5, 0};

const int FREQUENCY_PERIODS[MAX_FREQUENCY_INDICE - MIN_FREQUENCY_INDICE + 1] = {
    2024, 1911, 1803, 1702, 1607, 1516, 1431, 1351, 1275,
    1203, 1136, 1072, 1012, 955,  901,  851,  803,  758,
    715,  675,  637,  601,  568,  536,  506,
};

const float MELODY_BEATS[32] = {A, A, A, A, A, A, A, A, A, A, B, A, A, B, C, C,
                                C, C, A, A, C, C, C, C, A, A, A, A, B, A, A, B};

volatile unsigned char *DAC_ADDRESS = ((volatile unsigned char *)0x4000741C);

typedef enum {
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

typedef struct {
  Object super;
  int index;
  char buffer[128];

  MUSIC_PLAYER_STATE state;
} App;

typedef struct {
  Object super;

  bool is_playing;
  bool is_not_in_gap;

  int tone_index;

  int key;
  int tempo;
} MusicPlayer;

typedef struct {
  Object super;

  bool is_not_in_gap;
  bool is_muted;

  int frequency;
  int period;

  int wave_flip;

  int volume;
} ToneGenerator;

void print(char *format, int arg);
void print_raw(char *string);

void start_app(App *self, int unused);

void reader(App *self, int);
void receiver(App *self, int);

void player_tick(MusicPlayer *self, int);
void tone_tick(ToneGenerator *self);

// CAN
bool can_action(CANMsg *msg);
bool send_can_action(Can *self, CAN_ACTION can_action, char *data,
                     int data_length);

bool toggle_mute(ToneGenerator *self);
bool toggle_music_mute(MusicPlayer *self);
bool toggle_is_playing(ToneGenerator *self);

bool change_volume(ToneGenerator *self, int increment);
bool change_music_volume(MusicPlayer *self, int increment);
bool change_tempo(MusicPlayer *self, int bpm);
bool change_key(MusicPlayer *self, int key);

bool set_frequency(ToneGenerator *self, int frequency);

int get_period_from_frequency_indice(int indice);
void get_frequencies_with_key_offset(int offset, int *new_array);

App app = {initObject(), 0, {}, CONDUCTOR};
MusicPlayer music_player = {initObject(), 0, 0, 0, 120};
ToneGenerator tone_generator = {initObject(), 0, 0, 0, 0, 0, 10};

Serial sci0 = initSerial(SCI_PORT0, &app, reader);
Can can0 = initCan(CAN_PORT0, &app, receiver);

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
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

  print_raw("Welcome to the Music Player!\n");
  print_raw("Use the following commands to control the music:\n");
  print_raw("  - 'w': Increase the volume of the tone.\n");
  print_raw("  - 's': Decrease the volume of the tone.\n");
  print_raw("  - 'm': Toggle mute on/off for the tone.\n");
  print_raw("  - 't': Enter a new tempo (beats per minute).\n");
  print_raw("  - 'k': Enter a new key offset.\n");

  ASYNC(&music_player, player_tick, 0);
}

bool can_action(CANMsg *msg) {
  const int action_id = msg->msgId;

  const int data = atoi((char *)msg->buff);
  const int data_length = msg->length;

  print("Data: %d\n", data);
  print("Data length: %d\n", data_length);

  switch (action_id) {
  case STOP_MUSIC:

    break;
  case PLAY_MUSIC:
    break;
  case CHANGE_VOLUME:
    if (data_length > 0) {
      return SYNC(&music_player, change_music_volume, data);
    }

    break;
  case CHANGE_TEMPO:
    if (data_length > 0) {
      return SYNC(&music_player, change_tempo, data);
    }

    break;
  case CHANGE_KEY:
    if (data_length > 0) {
      return SYNC(&music_player, change_key, data);
    }

    break;
  case TOGGLE_MUTE:
    return SYNC(&music_player, toggle_music_mute, 0);

    break;
  case TOGGLE_IS_PLAYING:
    break;
  default:
    break;
  }

  return false;
}

bool send_can_action(Can *self, CAN_ACTION can_action, char *data,
                     int data_length) {
  if (data_length > 8) {
    return false;
  }

  CANMsg can_msg;

  for (int i = 0; i < data_length; i++) {
    can_msg.buff[i] = data[i];
  }

  can_msg.length = data_length;

  CAN_SEND(self, &can_msg);

  return true;
}

void receiver(App *self, int unused) {
  CANMsg msg;

  CAN_RECEIVE(&can0, &msg);

  if (!can_action(&msg)) {
    SCI_WRITE(&sci0, "Can action failed!\n");
  }
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
    if (send_can_action(&can0, CHANGE_VOLUME, "1", 1)) {
      print("Volume: %d\n", tone_generator.volume);
    } else {
      print_raw("Volume is already maximum.\n");
    }

    // if (SYNC(&music_player, change_music_volume, 1))
    //   print("Volume: %d\n", tone_generator.volume);
    // else
    //   print_raw("Volume is already maximum.\n");

    break;
  case 's':
    if (send_can_action(&can0, CHANGE_VOLUME, "-1", 2)) {
      print("Volume: %d\n", tone_generator.volume);
    } else {
      print_raw("Volume is already minimum.\n");
    }

    // if (SYNC(&music_player, change_music_volume, -1))
    //   print("Volume: %d\n", tone_generator.volume);
    // else
    //   print_raw("Volume is already minimum.\n");

    break;
  case 'm':
    print("Muted: %d\n", SYNC(&music_player, toggle_music_mute, 0));

    break;
  case 't':
    self->buffer[self->index] = '\0'; // Terminate string
    int bpm = atoi(self->buffer);     // Convert string to int
    self->index = 0;

    if (SYNC(&music_player, change_tempo, bpm))
      print("Tempo: %d\n", bpm);
    else
      print_raw("Tempo out of range\n");

    break;
  case 'k':                           // Process entered integer
    self->buffer[self->index] = '\0'; // Terminate string
    int new_key = atoi(self->buffer); // Convert string to int
    self->index = 0;                  // Reset buffer index

    if (SYNC(&music_player, change_key, new_key))
      print("Key: %d\n", new_key);
    else
      print_raw("Key out of range\n");

    break;
  default:
    if (!isdigit(c) && c != '-')
      return;

    print("Entered: %c\n", c);

    self->buffer[self->index++] = c; // store integer in buffer
  }
}

/**
 * Updates the tone output based on the given frequency period.
 * If the tone is not muted, it toggles the wave flip and sets the DAC address
 * accordingly. If the tone is muted, it sets the DAC address to 0. If the
 * deadline control is enabled, it schedules the next tone_tick event using
 * SEND. If the deadline control is disabled, it schedules the next tone_tick
 * event using AFTER.
 *
 * @param self The pointer to the App structure.
 * @param frequency_period The period of the frequency in microseconds.
 */
void tone_tick(ToneGenerator *self) {
  bool is_muted = self->is_muted;

  // Tone is muted, do not play. Go to next tick.
  if (!is_muted) {
    self->wave_flip ^= 1;

    int volume = self->volume;

    *DAC_ADDRESS = self->wave_flip * volume;
  } else
    *DAC_ADDRESS = 0;

  int frequency_period = get_period_from_frequency_indice(self->frequency);

  if (self->is_not_in_gap)
    SEND(USEC(frequency_period), USEC(100), self, tone_tick, 0);
}

bool toggle_music_mute(MusicPlayer *self) {
  bool muted = SYNC(&tone_generator, toggle_mute, 0);

  return muted;
}

/**
 * Toggles the mute state of the application.
 *
 * @param self A pointer to the App structure.
 */
bool toggle_mute(ToneGenerator *self) {
  self->is_muted = !self->is_muted;

  if (self->is_muted)
    *DAC_ADDRESS = 0;

  return self->is_muted;
}

bool change_music_volume(MusicPlayer *self, int increment) {
  bool success = SYNC(&tone_generator, change_volume, increment);

  return success;
}

/**
 * Changes the volume of the application.
 *
 * @param self A pointer to the App structure representing the application.
 * @param increment The amount by which to increment the volume.
 */
bool change_volume(ToneGenerator *self, int increment) {
  // Unmute if user tries to change the volume.
  if (self->is_muted)
    self->is_muted = false;

  if (self->volume + increment > MAX_VOLUME ||
      self->volume + increment < MIN_VOLUME)
    return false;

  self->volume += increment;

  return true;
}

bool change_tempo(MusicPlayer *self, int bpm) {
  if (bpm < MIN_TEMPO || bpm > MAX_TEMPO)
    return false;

  self->tempo = bpm;

  return true;
}

bool change_key(MusicPlayer *self, int key) {
  if (key > MAX_KEY || key < MIN_KEY)
    return false;

  self->key = key;

  return true;
}

bool set_frequency(ToneGenerator *self, int frequency) {
  self->frequency = frequency;

  return true;
}

bool toggle_is_playing(ToneGenerator *self) {
  self->is_not_in_gap = !self->is_not_in_gap;

  return self->is_not_in_gap;
}

void player_tick(MusicPlayer *self, int unused) {
  self->is_not_in_gap = SYNC(&tone_generator, toggle_is_playing, 0);

  if (self->is_not_in_gap) {
    int current_frequency =
        MELODY_FREQUENCY_INDICES[self->tone_index] + self->key;

    SYNC(&tone_generator, set_frequency, current_frequency);

    int beatMS = 60000 / (self->tempo * MELODY_BEATS[self->tone_index]);

    // ASYNC(&tone_generator, tone_tick, 0);

    // Go to next tone after the current beat.
    self->tone_index = (self->tone_index + 1) % 32;

    SEND(MSEC(beatMS - GAP_SILENCE), MSEC(beatMS), self, player_tick, 0);
  } else {
    SEND(MSEC(GAP_SILENCE), MSEC(GAP_SILENCE), self, player_tick, 0);
  }
}

/**
 * Gives the period based on the given frequency indice.
 *
 * @param indice The frequency indice.
 * @return The pre-calculated period.
 */
int get_period_from_frequency_indice(int indice) {
  return FREQUENCY_PERIODS[indice - MIN_FREQUENCY_INDICE];
}

/**
 * @brief
 *
 * This function takes an offset value and an array of integers as input. It
 * calculates the frequencies of each element in the array by adding the offset
 * to the elements value. The frequencies are stored in the provided new_array.
 *
 * @param offset The key offset value to be added to each element.
 * @param new_array The array to store the frequencies of elements.
 */
void get_frequencies_with_key_offset(int offset, int *new_array) {
  if (offset > MAX_KEY || offset < MIN_KEY)
    return;

  for (int i = 0; i < 32; i++) {
    int frequency_indice = MELODY_FREQUENCY_INDICES[i] + offset;

    new_array[i] = get_period_from_frequency_indice(frequency_indice);
  }
}