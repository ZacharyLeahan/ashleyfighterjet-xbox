#include "game.h"
#include <math.h>
#include <string.h>
static float clamp(float v, float a, float b) { return v < a ? a : v > b ? b : v; }
int circles_hit(float ax, float ay, float ar, float bx, float by, float br) {
    float dx = ax-bx, dy = ay-by, r = ar+br;
    return dx*dx + dy*dy <= r*r;
}
void game_init(Game *g) {
    memset(g, 0, sizeof(*g)); g->state = MENU; g->player_count = 1;
    for (int i=0; i<MAX_PLAYERS; ++i)
        g->players[i] = (Player){.x=i?420:320, .y=380, .health=3, .ammo=10};
}
void game_set_players(Game *g, int count) {
    /* Hotplug preserves each player's health, ammo and score within a run. */
    g->player_count = count >= 2 ? 2 : 1;
}
void game_step(Game *g, const Input inputs[MAX_PLAYERS], float dt) {
    int start = 0;
    for (int i=0; i<g->player_count; ++i) start |= inputs[i].start;
    int pressed = start && !g->start_held;
    g->start_held = start;
    if (g->state != PLAYING) {
        if (pressed) {
            int count = g->player_count;
            game_init(g); game_set_players(g,count);
            if (count==2) g->players[0].x=220;
            g->state = PLAYING; g->start_held = 1;
        }
        return;
    }
    dt = clamp(dt, 0, 0.05f);
    g->elapsed += dt; g->spawn_clock += dt;
    for (int i=0; i<g->player_count; ++i) {
        Player *p=&g->players[i]; Input in=inputs[i];
        if (!p->health) continue;
        /* Flick, then release to re-arm: unlimited rolls, no ammo cost. */
        float rx=clamp(in.roll_x,-1,1), ry=clamp(in.roll_y,-1,1);
        float strength=fmaxf(fabsf(rx),fabsf(ry));
        if (strength<0.25f) p->roll_held=0;
        if (strength>=0.65f && !p->roll_held) {
            p->roll_held=1;
            if (p->roll_clock<=0) {
                p->roll_clock=0.4f;
                p->roll_x=fabsf(rx)>=fabsf(ry)?(rx>0?1:-1):0;
                p->roll_y=p->roll_x?0:(ry>0?1:-1);
            }
        }
        /* The last tenth of a second jolts the jet in the chosen direction. */
        float burst=fminf(p->roll_clock,0.1f)-fminf(fmaxf(p->roll_clock-dt,0),0.1f);
        p->x=clamp(p->x+p->roll_x*720*burst,48,592);
        p->y=clamp(p->y+p->roll_y*720*burst,92,400);
        int rolling=p->roll_clock>0;
        p->roll_clock=fmaxf(p->roll_clock-dt,0);
        if (rolling) p->invincible=fmaxf(p->invincible,dt*2);
        float ix=clamp(in.x,-1,1), iy=clamp(in.y,-1,1);
        float length=sqrtf(ix*ix+iy*iy);
        if (length>1) { ix/=length; iy/=length; }
        p->x=clamp(p->x+ix*260*dt,48,592);
        p->y=clamp(p->y+iy*260*dt,92,400);
        p->shot_clock-=dt; p->invincible=clamp(p->invincible-dt,0,2);
        if (in.fire && p->ammo>0 && p->shot_clock<=0) {
            for (int b=0; b<MAX_BULLETS; ++b) if (!g->bullets[b].active) {
                g->bullets[b]=(Entity){.x=p->x,.y=p->y-32,.active=1,.owner=i};
                --p->ammo; p->shot_clock=0.22f; break;
            }
        }
    }
    if (g->spawned<LEVEL_ENEMIES && g->spawn_clock>=1.1f) {
        for (int i=0; i<MAX_ENEMIES; ++i) if (!g->enemies[i].active) {
            int n=g->spawned++;
            g->enemies[i]=(Entity){.x=80+(float)((n*173)%480),.y=-24,.active=1,.color=n%4};
            g->spawn_clock-=1.1f; break;
        }
    }
    for (int b=0; b<MAX_BULLETS; ++b) if (g->bullets[b].active) {
        Entity *shot=&g->bullets[b]; shot->y-=430*dt;
        if (shot->y<-10) shot->active=0;
        for (int e=0; shot->active && e<MAX_ENEMIES; ++e) {
            Entity *alien=&g->enemies[e];
            if (alien->active && circles_hit(shot->x,shot->y,4,alien->x,alien->y,20)) {
                shot->active=alien->active=0;
                g->players[shot->owner].score+=3; ++g->resolved;
            }
        }
    }
    for (int e=0; e<MAX_ENEMIES; ++e) if (g->enemies[e].active) {
        Entity *alien=&g->enemies[e]; alien->y+=105*dt;
        if (alien->y>510) {
            alien->active=0; ++g->resolved;
            for (int i=0; i<g->player_count; ++i) if (g->players[i].health) {
                ++g->players[i].score; ++g->players[i].ammo;
            }
        } else for (int i=0; i<g->player_count && alien->active; ++i) {
            Player *p=&g->players[i];
            if (p->health && p->invincible==0 && circles_hit(p->x,p->y,13,alien->x,alien->y,20)) {
                alien->active=0; ++g->resolved; --p->health; p->invincible=1.5f;
            }
        }
    }
    int alive=0;
    for (int i=0; i<g->player_count; ++i) alive |= g->players[i].health>0;
    if (!alive) g->state=LOST;
    else if (g->resolved==LEVEL_ENEMIES) g->state=WON;
}
