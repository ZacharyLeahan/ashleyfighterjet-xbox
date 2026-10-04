#ifndef SOUND_H
#define SOUND_H
#include <stdint.h>
#include "game.h"
#define SOUND_RATE 48000
/* Caller owns the returned signed mono PCM buffer. */
int16_t *sound_make(SoundEvent event, int *frames);
int16_t *sound_music(int *frames);
#endif
