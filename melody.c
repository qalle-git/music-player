#include "melody.h"

volatile unsigned char *DAC_ADDRESS = ((volatile unsigned char *)0x4000741C);

const int MELODY_FREQUENCY_INDICES[32] = {0, 2, 4, 0, 0, 2,  4, 0, 4,  5, 7,
                                          4, 5, 7, 7, 9, 7,  5, 4, 0,  7, 9,
                                          7, 5, 4, 0, 0, -5, 0, 0, -5, 0};

const int FREQUENCY_PERIODS[MAX_FREQUENCY_INDICE - MIN_FREQUENCY_INDICE + 1] = {
    2024, 1911, 1803, 1702, 1607, 1516, 1431, 1351, 1275,
    1203, 1136, 1072, 1012, 955,  901,  851,  803,  758,
    715,  675,  637,  601,  568,  536,  506,
};

const float MELODY_BEATS[32] = {
    BEAT_A, BEAT_A, BEAT_A, BEAT_A, BEAT_A, BEAT_A, BEAT_A, BEAT_A,
    BEAT_A, BEAT_A, BEAT_B, BEAT_A, BEAT_A, BEAT_B, BEAT_C, BEAT_C,
    BEAT_C, BEAT_C, BEAT_A, BEAT_A, BEAT_C, BEAT_C, BEAT_C, BEAT_C,
    BEAT_A, BEAT_A, BEAT_A, BEAT_A, BEAT_B, BEAT_A, BEAT_A, BEAT_B};

const int TWINKLE_MELODY_FREQUENCY_INDICES[32] = {
    0, 0, 7, 7, 9, 9, 7, 5, 5, 4, 4, 2, 2, 0, // "Twinkle, twinkle, little star"
    7, 7, 5, 5, 4, 4, 2,                      // "How I wonder what you are"
    7, 7, 5, 5, 4, 4, 2,                      // (repeat above line)
};

const float TWINKLE_MELODY_BEATS[32] = {
    BEAT_A, BEAT_A, BEAT_A, BEAT_A,
    BEAT_A, BEAT_A, BEAT_B, // "Twinkle, twinkle, little star"
    BEAT_A, BEAT_A, BEAT_A, BEAT_A,
    BEAT_A, BEAT_A, BEAT_B, // "How I wonder what you are"

    BEAT_A, BEAT_A, BEAT_A, BEAT_A,
    BEAT_A, BEAT_A, BEAT_B, // "Up above the world so high"
    BEAT_A, BEAT_A, BEAT_A, BEAT_A,
    BEAT_A, BEAT_A, BEAT_B // "Like a diamond in the sky"
};

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
