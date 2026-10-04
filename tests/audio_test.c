#include "audio.h"
#include "sound.h"
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    assert(SDL_setenv("SDL_VIDEODRIVER","dummy",1)==0);
    assert(SDL_setenv("SDL_AUDIODRIVER","dummy",1)==0);
    assert(SDL_Init(SDL_INIT_VIDEO)==0);
    assert(audio_init());
    /* Exercise overlapping effects, fanfare, and voice overflow
     * through the real SDL audio thread under UndefinedBehaviorSanitizer. */
    audio_play(SOUND_BOSS_DIE);audio_play(SOUND_WIN);
    for(int i=0;i<30;++i)audio_play(SOUND_SHOOT);
    audio_play(SOUND_CRASH);audio_play(SOUND_COUNT);
    SDL_Delay(3600);
    audio_close();audio_close();
    assert(audio_init());audio_play(SOUND_EMPTY);SDL_Delay(150);
    audio_close();SDL_Quit();
    puts("PASS: SDL playback, overlapping sounds, voice overflow, cleanup/reopen");
}
