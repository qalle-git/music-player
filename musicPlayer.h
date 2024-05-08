#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "TinyTimber.h"
#include <stdbool.h>

typedef struct {
  Object super;

  bool is_playing;
  bool is_not_in_gap;

  int tone_index;

  int key;
  int tempo;
} MusicPlayer;

bool start_music(MusicPlayer *self);
bool stop_music(MusicPlayer *self);

void player_tick(MusicPlayer *self, int);

bool toggle_music_mute(MusicPlayer *self);

int change_music_volume(MusicPlayer *self, int increment);
bool change_tempo(MusicPlayer *self, int bpm);
bool change_key(MusicPlayer *self, int key);

extern MusicPlayer music_player;

#endif