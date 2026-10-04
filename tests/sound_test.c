#include "sound.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void check_pcm(const int16_t *pcm,int frames) {
    assert(pcm&&frames>0);
    long long energy=0;
    for(int i=0;i<frames;++i) { assert(abs(pcm[i])<32767);energy+=(long long)pcm[i]*pcm[i]; }
    assert(energy/frames>1000);
}
int main(void) {
    for(int i=0;i<SOUND_COUNT;++i) {
        int frames;int16_t *pcm=sound_make((SoundEvent)i,&frames);
        check_pcm(pcm,frames);assert(abs(pcm[frames-1])<50);free(pcm);
    }
    int frames;int16_t *music=sound_music(&frames);check_pcm(music,frames);
    assert(frames==SOUND_RATE*16&&abs(music[0]-music[frames-1])<500);free(music);
    assert(sound_make(SOUND_COUNT,&frames)==NULL&&frames==0);
    puts("PASS: synthesized sound PCM, music seam, invalid effects");
    return 0;
}
