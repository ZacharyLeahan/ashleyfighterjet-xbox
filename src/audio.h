#ifndef AUDIO_H
#define AUDIO_H
#include "game.h"
int audio_init(void);
void audio_play(SoundEvent event);
void audio_close(void);
#endif
