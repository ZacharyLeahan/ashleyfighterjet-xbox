#include "audio.h"
#include "sound.h"
#include <SDL.h>
#include <stdlib.h>
#define VOICES 24
typedef struct { int16_t *pcm; int frames; } Clip;
typedef struct { int sound,position; } Voice;
static SDL_AudioDeviceID device;
static Clip clips[SOUND_COUNT],music;
static Voice voices[VOICES];
static int music_position,duck_frames;
static void callback(void *userdata,Uint8 *stream,int bytes) {
    (void)userdata;Sint16 *out=(Sint16 *)stream;
    for(int i=0;i<bytes/(int)(2*sizeof(*out));++i) {
        int sample=music.pcm[music_position++];
        if(music_position>=music.frames)music_position=0;
        if(duck_frames>0) {
            int total=SOUND_RATE*5/2;
            sample=sample*(10-(duck_frames*7/total))/10;--duck_frames;
        }
        for(int v=0;v<VOICES;++v)if(voices[v].sound>=0) {
            Clip *clip=&clips[voices[v].sound];sample+=clip->pcm[voices[v].position++];
            if(voices[v].position>=clip->frames)voices[v].sound=-1;
        }
        if(sample>32767)sample=32767;if(sample<-32768)sample=-32768;
        out[i*2]=out[i*2+1]=(Sint16)sample;
    }
}
void audio_close(void) {
    if(device)SDL_CloseAudioDevice(device);device=0;
    for(int i=0;i<SOUND_COUNT;++i){free(clips[i].pcm);clips[i]=(Clip){0};}
    free(music.pcm);music=(Clip){0};
}
int audio_init(void) {
    if(device)return 1;
    if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0)return 0;
    for(int i=0;i<SOUND_COUNT;++i) {
        clips[i].pcm=sound_make((SoundEvent)i,&clips[i].frames);
        if(!clips[i].pcm){audio_close();SDL_SetError("Sound synthesis allocation failed");return 0;}
    }
    music.pcm=sound_music(&music.frames);
    if(!music.pcm){audio_close();SDL_SetError("Music synthesis allocation failed");return 0;}
    for(int i=0;i<VOICES;++i)voices[i]=(Voice){.sound=-1};
    music_position=duck_frames=0;
    SDL_AudioSpec want={0};want.freq=SOUND_RATE;want.format=AUDIO_S16SYS;
    want.channels=2;want.samples=1024;want.callback=callback;
    device=SDL_OpenAudioDevice(NULL,0,&want,NULL,0);
    if(!device){audio_close();return 0;}
    SDL_PauseAudioDevice(device,0);return 1;
}
void audio_play(SoundEvent event) {
    if(!device||event<0||event>=SOUND_COUNT)return;
    SDL_LockAudioDevice(device);
    if(event==SOUND_CRASH)duck_frames=SOUND_RATE*5/2;
    int slot=-1;
    for(int i=0;i<VOICES;++i)if(voices[i].sound<0){slot=i;break;}
    if(slot<0)for(int i=0;i<VOICES;++i)if(voices[i].sound==SOUND_SHOOT||voices[i].sound==SOUND_POP){slot=i;break;}
    if(slot>=0)voices[slot]=(Voice){.sound=event,.position=0};
    SDL_UnlockAudioDevice(device);
}
