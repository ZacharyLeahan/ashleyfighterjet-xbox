#include "game.h"
#include <math.h>
#include <string.h>
static float clamp(float v, float a, float b) { return v < a ? a : v > b ? b : v; }
int circles_hit(float ax, float ay, float ar, float bx, float by, float br) {
    float dx = ax-bx, dy = ay-by, r = ar+br;
    return dx*dx + dy*dy <= r*r;
}
void game_init(Game *g) {
    memset(g, 0, sizeof(*g)); g->state = MENU;
    g->x = 320; g->y = 380; g->health = 3; g->ammo = 10;
}
void game_step(Game *g, Input in, float dt) {
    int pressed = in.start && !g->start_held;
    g->start_held = in.start;
    if (g->state != PLAYING) {
        if (pressed) { game_init(g); g->state = PLAYING; g->start_held = 1; }
        return;
    }
    dt = clamp(dt, 0, 0.05f);
    float ix = clamp(in.x,-1,1), iy = clamp(in.y,-1,1);
    float length = sqrtf(ix*ix+iy*iy);
    if (length > 1) { ix /= length; iy /= length; }
    g->x = clamp(g->x + ix*260*dt, 48, 592);
    g->y = clamp(g->y + iy*260*dt, 92, 400);
    g->elapsed += dt; g->spawn_clock += dt; g->shot_clock -= dt;
    g->invincible = clamp(g->invincible-dt, 0, 2);
    if (g->spawned < LEVEL_ENEMIES && g->spawn_clock >= 1.1f) {
        for (int i=0; i<MAX_ENEMIES; ++i) if (!g->enemies[i].active) {
            int n = g->spawned++;
            g->enemies[i] = (Entity){80+(float)((n*173)%480), -24, 1, n%4};
            g->spawn_clock -= 1.1f; break;
        }
    }
    if (in.fire && g->ammo > 0 && g->shot_clock <= 0) {
        for (int i=0; i<MAX_BULLETS; ++i) if (!g->bullets[i].active) {
            g->bullets[i] = (Entity){g->x, g->y-32, 1, 0};
            --g->ammo; g->shot_clock = 0.22f; break;
        }
    }
    for (int b=0; b<MAX_BULLETS; ++b) if (g->bullets[b].active) {
        Entity *shot = &g->bullets[b]; shot->y -= 430*dt;
        if (shot->y < -10) shot->active = 0;
        for (int e=0; shot->active && e<MAX_ENEMIES; ++e) {
            Entity *alien = &g->enemies[e];
            if (alien->active && circles_hit(shot->x,shot->y,4,alien->x,alien->y,20)) {
                shot->active = alien->active = 0; g->score += 3; ++g->resolved;
            }
        }
    }
    for (int e=0; e<MAX_ENEMIES; ++e) if (g->enemies[e].active) {
        Entity *alien = &g->enemies[e]; alien->y += 105*dt;
        if (alien->y > 510) { alien->active = 0; ++g->resolved; ++g->score; ++g->ammo; }
        else if (g->invincible == 0 && circles_hit(g->x,g->y,13,alien->x,alien->y,20)) {
            alien->active = 0; ++g->resolved; --g->health; g->invincible = 1.5f;
            if (!g->health) { g->state = LOST; return; }
        }
    }
    if (g->resolved == LEVEL_ENEMIES) g->state = WON;
}
