#include "game.h"
#include <assert.h>
#include <stdio.h>
static void start(Game *g) { game_init(g);game_step(g,(Input){.start=1},1.0f/60);game_step(g,(Input){0},1.0f/60); }
int main(void) {
    assert(circles_hit(0,0,5,10,0,5));assert(!circles_hit(0,0,5,10.01f,0,5));
    Game g;start(&g);assert(g.state==PLAYING&&g.ammo==10&&g.health==3);
    for(int i=0;i<1000;++i)game_step(&g,(Input){.x=1,.y=1},1.0f/60);
    assert(g.x<=592&&g.y<=400);
    start(&g);game_step(&g,(Input){.fire=1},1.0f/60);assert(g.ammo==9);
    game_step(&g,(Input){.fire=1},1.0f/60);assert(g.ammo==9);
    start(&g);g.enemies[0]=(Entity){100,509,1,0};g.spawned=1;
    game_step(&g,(Input){0},1.0f/60);assert(g.score==1&&g.ammo==11&&g.resolved==1);
    start(&g);g.enemies[0]=(Entity){200,100,1,0};g.bullets[0]=(Entity){200,107,1,0};g.spawned=1;
    game_step(&g,(Input){0},1.0f/60);assert(g.score==3&&g.resolved==1&&!g.bullets[0].active);
    start(&g);g.enemies[0]=(Entity){g.x,g.y,1,0};g.spawned=1;
    game_step(&g,(Input){0},1.0f/60);assert(g.health==2&&g.invincible>0);
    g.enemies[1]=(Entity){g.x,g.y,1,0};game_step(&g,(Input){0},1.0f/60);assert(g.health==2);
    start(&g);g.health=1;g.enemies[0]=(Entity){g.x,g.y,1,0};game_step(&g,(Input){0},1.0f/60);assert(g.state==LOST);
    game_step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.health==3);
    /* Stay in an empty lane: all 24 scheduled aliens must resolve and win. */
    start(&g);g.x=592;g.y=92;
    for(int i=0;i<2400&&g.state==PLAYING;++i)game_step(&g,(Input){0},1.0f/60);
    assert(g.state==WON&&g.spawned==24&&g.resolved==24&&g.score==24&&g.ammo==34);
    game_step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.score==0);
    puts("PASS: collisions, movement bounds, firing, dodge rewards, damage, waves, win, restart");
}
