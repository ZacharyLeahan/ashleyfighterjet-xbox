#include <SDL.h>
#include "game.h"
#include "render.h"
#include "audio.h"
#ifndef HOST_BUILD
#include <hal/video.h>
#include <hal/debug.h>
#include <windows.h>
#endif
static float axis(Sint16 v) {
    float n=v/32767.0f;
    if(n>-0.18f&&n<0.18f)return 0;
    return n>0?(n-0.18f)/0.82f:(n+0.18f)/0.82f;
}
int main(void) {
#ifndef HOST_BUILD
    XVideoSetMode(640,480,32,REFRESH_DEFAULT);
#endif
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER)<0)goto fail;
    SDL_Window *w=SDL_CreateWindow("Ashley Fighter Jet - Xbox prototype",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,480,SDL_WINDOW_SHOWN);
    SDL_Renderer *r=w?SDL_CreateRenderer(w,-1,0):NULL;
    if(!r)goto fail;
    SDL_RenderSetLogicalSize(r,640,480);
    if(!audio_init())SDL_Log("Audio unavailable: %s",SDL_GetError());
    SDL_GameController *pads[MAX_PLAYERS]={0};
    Game g; game_init(&g);
    Uint32 previous=SDL_GetTicks(); float accumulator=0,time=0;
    int running=1, pending_start[MAX_PLAYERS]={0}, pending_fire[MAX_PLAYERS]={0};
    while(running) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT)running=0;
            if(e.type==SDL_KEYDOWN&&!e.key.repeat) {
                pending_start[0]|=e.key.keysym.scancode==SDL_SCANCODE_RETURN;
                pending_fire[0]|=e.key.keysym.scancode==SDL_SCANCODE_SPACE;
            }
            if(e.type==SDL_CONTROLLERBUTTONDOWN) for(int i=0;i<MAX_PLAYERS;++i) {
                if(pads[i]&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i]))==e.cbutton.which) {
                    pending_start[i]|=e.cbutton.button==SDL_CONTROLLER_BUTTON_START;
                    pending_fire[i]|=e.cbutton.button==SDL_CONTROLLER_BUTTON_A;
                }
            }
        }
        for(int i=0;i<MAX_PLAYERS;++i) if(pads[i]&&!SDL_GameControllerGetAttached(pads[i])) {
            SDL_GameControllerClose(pads[i]); pads[i]=NULL;
            pending_start[i]=pending_fire[i]=0;
        }
        /* Keep the remaining controller and its pilot together after unplugging P1. */
        if(!pads[0]&&pads[1]) {
            pads[0]=pads[1]; pads[1]=NULL;
            Player saved=g.players[0]; g.players[0]=g.players[1]; g.players[1]=saved;
            for(int b=0;b<MAX_BULLETS;++b) if(g.bullets[b].active)g.bullets[b].owner=1-g.bullets[b].owner;
            pending_start[0]=pending_start[1]; pending_fire[0]=pending_fire[1];
            pending_start[1]=pending_fire[1]=0;
        }
        /* Enumerate connected devices, accepting the first two supported controllers. */
        for(int device=0;device<SDL_NumJoysticks();++device) {
            if(!SDL_IsGameController(device))continue;
            SDL_JoystickID id=SDL_JoystickGetDeviceInstanceID(device); int known=0;
            for(int i=0;i<MAX_PLAYERS;++i)if(pads[i]&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pads[i]))==id)known=1;
            if(known)continue;
            for(int i=0;i<MAX_PLAYERS;++i)if(!pads[i]) { pads[i]=SDL_GameControllerOpen(device); break; }
        }
        game_set_players(&g,pads[1]?2:1);
        Input in[MAX_PLAYERS]={{0}}; const Uint8 *keys=SDL_GetKeyboardState(NULL);
        in[0].x=(float)(keys[SDL_SCANCODE_RIGHT]-keys[SDL_SCANCODE_LEFT]);
        in[0].y=(float)(keys[SDL_SCANCODE_DOWN]-keys[SDL_SCANCODE_UP]);
        in[0].fire=keys[SDL_SCANCODE_SPACE]; in[0].start=keys[SDL_SCANCODE_RETURN];
        for(int i=0;i<MAX_PLAYERS;++i)if(pads[i]) {
            in[i].roll_x=SDL_GameControllerGetAxis(pads[i],SDL_CONTROLLER_AXIS_RIGHTX)/32767.0f;
            in[i].roll_y=SDL_GameControllerGetAxis(pads[i],SDL_CONTROLLER_AXIS_RIGHTY)/32767.0f;
            in[i].x+=axis(SDL_GameControllerGetAxis(pads[i],SDL_CONTROLLER_AXIS_LEFTX));
            in[i].y+=axis(SDL_GameControllerGetAxis(pads[i],SDL_CONTROLLER_AXIS_LEFTY));
            in[i].x+=SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_DPAD_RIGHT)-SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            in[i].y+=SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_DPAD_DOWN)-SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_DPAD_UP);
            in[i].fire|=SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_A);
            in[i].fire|=SDL_GameControllerGetAxis(pads[i],SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>8192;
            in[i].start|=SDL_GameControllerGetButton(pads[i],SDL_CONTROLLER_BUTTON_START);
        }
        Uint32 now=SDL_GetTicks(); float elapsed=(now-previous)/1000.0f;previous=now;
        if(elapsed>0.1f)elapsed=0.1f;
        accumulator+=elapsed;time+=elapsed;
        while(accumulator>=1.0f/60) {
            Input step[MAX_PLAYERS]={in[0],in[1]};
            for(int i=0;i<MAX_PLAYERS;++i) {
                step[i].start|=pending_start[i]; step[i].fire|=pending_fire[i];
                pending_start[i]=pending_fire[i]=0;
            }
            game_step(&g,step,1.0f/60);
            for(int i=0;i<g.sound_count;++i)audio_play(g.sounds[i]);
            accumulator-=1.0f/60;
        }
        draw_game(r,&g,time);SDL_Delay(1);
    }
    for(int i=0;i<MAX_PLAYERS;++i)if(pads[i])SDL_GameControllerClose(pads[i]);
    audio_close();
    SDL_DestroyRenderer(r);SDL_DestroyWindow(w);SDL_Quit();return 0;
fail:
#ifndef HOST_BUILD
    debugPrint("Ashley Fighter Jet error: %s\n",SDL_GetError());for(;;)Sleep(1000);
#else
    SDL_Log("Ashley Fighter Jet error: %s",SDL_GetError());return 1;
#endif
}
