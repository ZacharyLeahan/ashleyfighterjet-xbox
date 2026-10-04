/* Port of Ashley's HTML5 Web Audio recipes; no external audio assets. */
#include "sound.h"
#include <math.h>
#include <stdlib.h>
#define PI 3.14159265358979323846f
#define TAU (2*PI)
typedef enum { SINE, SQUARE, TRIANGLE, SAW, NOISE } Wave;
typedef struct { float b0,b1,b2,a1,a2,x1,x2,y1,y2; } Filter;
static Filter filter_make(float hz,int bandpass) {
    float w=TAU*hz/SOUND_RATE,c=cosf(w),a=sinf(w)/(bandpass?2:1.41421356f);
    float norm=1/(1+a);
    Filter f={0};
    f.b0=(bandpass?a:(1-c)/2)*norm;
    f.b1=bandpass?0:(1-c)*norm;f.b2=bandpass?-a*norm:f.b0;
    f.a1=-2*c*norm;f.a2=(1-a)*norm;return f;
}
static float filtered(Filter *f,float x) {
    float y=f->b0*x+f->b1*f->x1+f->b2*f->x2-f->a1*f->y1-f->a2*f->y2;
    f->x2=f->x1;f->x1=x;f->y2=f->y1;f->y1=y;return y;
}
static float note(int midi) { return 440*powf(2,(midi-69)/12.0f); }
/* Exponential pitch/gain ramps match the original oscillator recipes.
 * Negative attack selects a slow pad envelope rather than a one-shot. */
static void layer(float *out,int frames,float at,float duration,Wave wave,
                  float f0,float f1,float sweep,float gain,float decay,
                  float attack,float cutoff,int bandpass,int loop,int wobble) {
    int length=(int)(duration*SOUND_RATE),offset=(int)(at*SOUND_RATE);
    float phase=0,freq=f0;
    float freq_step=sweep>0?powf(f1/f0,1/(sweep*SOUND_RATE)):1;
    float env=gain,env_step=powf(0.001f/gain,1/(fmaxf(0.001f,decay-fmaxf(0,attack))*SOUND_RATE));
    Filter filter=filter_make(cutoff>0?cutoff:1000,bandpass);
    uint32_t rng=0x735a12u+(uint32_t)offset;
    for(int i=0;i<length;++i) {
        float t=i/(float)SOUND_RATE,value;
        if(wave==NOISE) {
            rng=rng*1664525u+1013904223u;value=(rng>>8)/8388608.0f-1;
        } else {
            float hz=freq+(wobble?22*sinf(TAU*9*t):0);
            phase+=hz/SOUND_RATE;phase-=floorf(phase);
            value=wave==SINE?sinf(TAU*phase):wave==SQUARE?(phase<0.5f?1:-1):
                  wave==TRIANGLE?1-4*fabsf(phase-0.5f):2*phase-1;
        }
        if(cutoff>0)value=filtered(&filter,value);
        float volume;
        if(attack<0) {
            volume=gain*fminf(1,t/1.4f)*fminf(1,fmaxf(0,(duration-t)/1.6f));
        } else {
            volume=attack>0&&t<attack?gain*t/attack:env;
            if(t>=attack)env*=env_step;
        }
        /* Remove hard-stop clicks while retaining the source's decay. */
        volume*=fminf(1,(duration-t)/0.005f);
        int dest=offset+i;
        if(loop)dest%=frames;
        if(dest>=0&&dest<frames)out[dest]+=value*volume;
        if(t<sweep)freq*=freq_step;
    }
}
static int16_t *finish(float *mix,int frames,float volume) {
    int16_t *pcm=malloc((size_t)frames*sizeof(*pcm));
    if(pcm)for(int i=0;i<frames;++i) {
        float s=mix[i]*volume; s=fmaxf(-1,fminf(1,s));pcm[i]=(int16_t)(s*32767);
    }
    free(mix);return pcm;
}
static void melody(float *out,int frames,int midi,float at,float dur,float gain,Wave wave) {
    layer(out,frames,at,dur+0.05f,wave,note(midi),note(midi),0,gain,dur,0.02f,0,0,0,0);
}
int16_t *sound_make(SoundEvent event,int *frames) {
    static const float lengths[SOUND_COUNT]={0.15f,0.23f,0.10f,0.53f,0.18f,0.17f,1.16f,2.3f,1.25f};
    *frames=0;if(event<0||event>=SOUND_COUNT)return NULL;
    int n=(int)(lengths[event]*SOUND_RATE);float *out=calloc((size_t)n,sizeof(*out));
    if(!out)return NULL;
    switch(event) {
    case SOUND_SHOOT:layer(out,n,0,.15f,SQUARE,880,180,.13f,.11f,.14f,0,0,0,0,0);break;
    case SOUND_POP:
        layer(out,n,0,.23f,SINE,320,900,.09f,.22f,.22f,0,0,0,0,0);
        layer(out,n,0,.13f,NOISE,1,1,0,.1f,.12f,0,2600,1,0,0);break;
    case SOUND_EMPTY:layer(out,n,0,.1f,TRIANGLE,140,90,.08f,.12f,.09f,0,0,0,0,0);break;
    case SOUND_ALARM:
        for(int i=0;i<2;++i)layer(out,n,i*.28f,.25f,SQUARE,110,110,0,.14f,.24f,.02f,0,0,0,0);
        break;
    case SOUND_BOSS_SHOOT:layer(out,n,0,.18f,SINE,400,150,.16f,.08f,.17f,0,0,0,0,0);break;
    case SOUND_BOSS_HIT:
        layer(out,n,0,.17f,SINE,170,55,.14f,.25f,.16f,0,0,0,0,0);
        layer(out,n,0,.09f,NOISE,1,1,0,.08f,.08f,0,800,1,0,0);break;
    case SOUND_BOSS_DIE: {
        layer(out,n,0,.5f,NOISE,1,1,0,.5f,.5f,0,320,0,0,0);
        layer(out,n,0,.8f,SAW,200,40,.7f,.28f,.75f,0,0,0,0,0);
        const int notes[]={72,76,79,84};
        for(int i=0;i<4;++i)melody(out,n,notes[i],.4f+i*.12f,.35f,.12f,TRIANGLE);
        break;
    }
    case SOUND_WIN: {
        const int notes[]={60,64,67,72},sparkles[]={84,88,91,96};
        for(int i=0;i<4;++i) {
            melody(out,n,notes[i],i*.15f,.3f,.13f,TRIANGLE);
            melody(out,n,notes[i],.7f,1.5f,.07f,TRIANGLE);
            melody(out,n,sparkles[i],.9f+i*.18f,.5f,.05f,SINE);
        }
        break;
    }
    case SOUND_CRASH:
        layer(out,n,0,1.25f,SAW,300,55,1,.3f,1.2f,0,900,0,0,1);
        layer(out,n,0,.36f,NOISE,1,1,0,.5f,.35f,0,220,0,0,0);break;
    default:break;
    }
    int16_t *pcm=finish(out,n,.9f);if(pcm)*frames=n;return pcm;
}
int16_t *sound_music(int *frames) {
    int n=SOUND_RATE*16;*frames=0;float *out=calloc((size_t)n,sizeof(*out));if(!out)return NULL;
    const int chords[4][4]={{45,57,60,64},{41,53,57,60},{48,55,60,64},{43,55,59,62}};
    const int twinkles[]={69,72,74,76,79,81,84};
    for(int bar=0;bar<4;++bar) {
        for(int voice=0;voice<4;++voice)for(int det=-1;det<=1;det+=2) {
            float hz=note(chords[bar][voice])*powf(2,det*5/1200.0f);
            layer(out,n,bar*4,5.2f,TRIANGLE,hz,hz,0,voice?.032f:.05f,0,-1,1100,0,1,0);
        }
        float hz=note(chords[bar][0]-12);
        layer(out,n,bar*4,4.6f,SINE,hz,hz,0,.07f,0,-1,0,0,1,0);
        for(int i=0;i<3;++i) {
            hz=note(twinkles[(bar*3+i)%7]);
            layer(out,n,bar*4+.5f+i*1.05f,.9f,SINE,hz,hz,0,.035f,.8f,.02f,0,0,1,0);
        }
    }
    int16_t *pcm=finish(out,n,.5f);if(pcm)*frames=n;return pcm;
}
