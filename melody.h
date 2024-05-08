#ifndef MELODY_H
#define MELODY_H

// #define TWINKLE

#define MIN_VOLUME 1
#define MAX_VOLUME 25

#define MIN_FREQUENCY_INDICE -10
#define MAX_FREQUENCY_INDICE 14

#define MIN_KEY -5
#define MAX_KEY 5

#define MIN_TEMPO 60
#define MAX_TEMPO 240

#define GAP_SILENCE 85

#define BEAT_A (1.)
#define BEAT_B (BEAT_A / 2.)
#define BEAT_C (BEAT_A * 2.)

extern volatile unsigned char *DAC_ADDRESS;

extern const int MELODY_FREQUENCY_INDICES[32];
extern const int
    FREQUENCY_PERIODS[MAX_FREQUENCY_INDICE - MIN_FREQUENCY_INDICE + 1];
extern const float MELODY_BEATS[32];

extern const int TWINKLE_MELODY_FREQUENCY_INDICES[32];
extern const float TWINKLE_MELODY_BEATS[32];

int get_period_from_frequency_indice(int indice);
void get_frequencies_with_key_offset(int offset, int *new_array);

#endif