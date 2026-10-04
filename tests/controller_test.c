/* Exercise the real SDL event/input loop using host-only virtual controllers. */
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include "game.h"
int game_main(void);
static SDL_Joystick *sticks[3];
static int devices[3], phase;
static Uint32 began;
static void button(int player, int button, int pressed) {
    assert(SDL_JoystickSetVirtualButton(sticks[player],button,(Uint8)pressed)==0);
}
void draw_game(SDL_Renderer *r,const Game *g,float t) {
    (void)r;(void)t;
    assert(SDL_GetTicks()-began<5000);
    if(phase==0) {
        assert(g->player_count==2);button(1,SDL_CONTROLLER_BUTTON_START,1);phase=1;
    } else if(phase==1&&g->state==PLAYING) {
        button(1,SDL_CONTROLLER_BUTTON_START,0);
        button(0,SDL_CONTROLLER_BUTTON_A,1);button(1,SDL_CONTROLLER_BUTTON_A,1);
        assert(SDL_JoystickSetVirtualAxis(sticks[0],SDL_CONTROLLER_AXIS_LEFTX,-32767)==0);
        assert(SDL_JoystickSetVirtualAxis(sticks[1],SDL_CONTROLLER_AXIS_LEFTX,32767)==0);
        assert(SDL_JoystickSetVirtualAxis(sticks[0],SDL_CONTROLLER_AXIS_RIGHTX,-32767)==0);
        assert(SDL_JoystickSetVirtualAxis(sticks[1],SDL_CONTROLLER_AXIS_RIGHTY,-32767)==0);
        phase=2;
    } else if(phase==2&&g->players[0].ammo==9&&g->players[1].ammo==9) {
        assert(g->players[0].x<220&&g->players[1].x>420);
        assert(g->players[0].roll_clock>0&&g->players[0].roll_x==-1);
        assert(g->players[1].roll_clock>0&&g->players[1].roll_y==-1);
        button(0,SDL_CONTROLLER_BUTTON_A,0);button(1,SDL_CONTROLLER_BUTTON_A,0);
        assert(SDL_JoystickDetachVirtual(devices[2])==0);
        assert(SDL_JoystickDetachVirtual(devices[0])==0);phase=3;
    } else if(phase==3&&g->player_count==1) {
        /* Remaining pilot retains its jet, ammo and shots after promotion. */
        assert(g->players[0].x>420&&g->players[0].ammo==9);
        int owned=0;
        for(int b=0;b<MAX_BULLETS;++b)if(g->bullets[b].active&&g->bullets[b].x>420){assert(g->bullets[b].owner==0);owned=1;}
        assert(owned);
        devices[0]=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,6,15,0);
        assert(devices[0]>=0);phase=4;
    } else if(phase==4&&g->player_count==2) {
        assert(g->players[0].ammo==9&&g->players[1].ammo==9);
        SDL_Event quit={.type=SDL_QUIT};assert(SDL_PushEvent(&quit)==1);phase=5;
    }
}
int main(void) {
    assert(SDL_setenv("SDL_VIDEODRIVER","dummy",1)==0);
    assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER)==0);
    began=SDL_GetTicks();
    for(int i=0;i<3;++i) {
        devices[i]=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,6,15,0);
        assert(devices[i]>=0&&SDL_IsGameController(devices[i]));
        sticks[i]=SDL_JoystickOpen(devices[i]);assert(sticks[i]);
    }
    assert(game_main()==0&&phase==5);
    puts("PASS: two controllers, P2 Start, separate movement/fire, third ignored, unplug/replug, shot ownership");
    return 0;
}
