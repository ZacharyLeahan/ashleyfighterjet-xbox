#include <SDL.h>
#include "game.h"
#include "render.h"
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
    SDL_GameController *pad=NULL;
    for(int i=0;i<SDL_NumJoysticks();++i)if(SDL_IsGameController(i)){pad=SDL_GameControllerOpen(i);if(pad)break;}
    Game g; game_init(&g);
    Uint32 previous=SDL_GetTicks(); float accumulator=0,time=0; int running=1, pending_start=0, pending_fire=0;
    while(running) {
        SDL_Event e; int start_tapped=0, fire_tapped=0;
        while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT)running=0;
            if(e.type==SDL_KEYDOWN&&!e.key.repeat) { start_tapped|=e.key.keysym.scancode==SDL_SCANCODE_RETURN; fire_tapped|=e.key.keysym.scancode==SDL_SCANCODE_SPACE; }
            if(e.type==SDL_CONTROLLERBUTTONDOWN) { start_tapped|=e.cbutton.button==SDL_CONTROLLER_BUTTON_START; fire_tapped|=e.cbutton.button==SDL_CONTROLLER_BUTTON_A; }
            if(e.type==SDL_CONTROLLERDEVICEADDED&&!pad)pad=SDL_GameControllerOpen(e.cdevice.which);
            if(e.type==SDL_CONTROLLERDEVICEREMOVED&&pad&&SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad))==e.cdevice.which){SDL_GameControllerClose(pad);pad=NULL;}
        }
        Input in={0}; const Uint8 *keys=SDL_GetKeyboardState(NULL);
        in.x=(float)(keys[SDL_SCANCODE_RIGHT]-keys[SDL_SCANCODE_LEFT]);
        in.y=(float)(keys[SDL_SCANCODE_DOWN]-keys[SDL_SCANCODE_UP]);
        in.fire=keys[SDL_SCANCODE_SPACE]; in.start=keys[SDL_SCANCODE_RETURN];
        if(pad) {
            in.x+=axis(SDL_GameControllerGetAxis(pad,SDL_CONTROLLER_AXIS_LEFTX));
            in.y+=axis(SDL_GameControllerGetAxis(pad,SDL_CONTROLLER_AXIS_LEFTY));
            in.x+=SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_RIGHT)-SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            in.y+=SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_DOWN)-SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_UP);
            in.fire|=SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_A);
            in.start|=SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_START);
        }
        pending_start|=start_tapped;pending_fire|=fire_tapped;
        Uint32 now=SDL_GetTicks(); float elapsed=(now-previous)/1000.0f;previous=now;
        if(elapsed>0.1f)elapsed=0.1f;
        accumulator+=elapsed;time+=elapsed;
        while(accumulator>=1.0f/60){Input step=in;step.start|=pending_start;step.fire|=pending_fire;game_step(&g,step,1.0f/60);pending_start=pending_fire=0;accumulator-=1.0f/60;}
        draw_game(r,&g,time);SDL_Delay(1);
    }
    if(pad)SDL_GameControllerClose(pad);
    SDL_DestroyRenderer(r);SDL_DestroyWindow(w);SDL_Quit();return 0;
fail:
#ifndef HOST_BUILD
    debugPrint("Ashley Fighter Jet error: %s\n",SDL_GetError());for(;;)Sleep(1000);
#else
    SDL_Log("Ashley Fighter Jet error: %s",SDL_GetError());return 1;
#endif
}
