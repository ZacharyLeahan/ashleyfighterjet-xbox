#include "game.h"
#include <assert.h>
#include <stdio.h>
static void step(Game *g, Input in, float dt) { Input inputs[MAX_PLAYERS]={in,{0}};game_step(g,inputs,dt); }
static void start(Game *g) { game_init(g);step(g,(Input){.start=1},1.0f/60);step(g,(Input){0},1.0f/60); }
int main(void) {
    assert(circles_hit(0,0,5,10,0,5));assert(!circles_hit(0,0,5,10.01f,0,5));
    Game g;start(&g);assert(g.state==PLAYING&&g.players[0].ammo==10&&g.players[0].health==3);
    for(int i=0;i<1000;++i)step(&g,(Input){.x=1,.y=1},1.0f/60);
    assert(g.players[0].x<=592&&g.players[0].y<=400);
    start(&g);step(&g,(Input){.fire=1},1.0f/60);assert(g.players[0].ammo==9);
    step(&g,(Input){.fire=1},1.0f/60);assert(g.players[0].ammo==9);
    start(&g);g.enemies[0]=(Entity){.x=100,.y=509,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].score==1&&g.players[0].ammo==11&&g.resolved==1);
    start(&g);g.enemies[0]=(Entity){.x=200,.y=100,.active=1};g.bullets[0]=(Entity){.x=200,.y=107,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].score==3&&g.resolved==1&&!g.bullets[0].active);
    start(&g);g.enemies[0]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].health==2&&g.players[0].invincible>0);
    g.enemies[1]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};step(&g,(Input){0},1.0f/60);assert(g.players[0].health==2);
    start(&g);g.players[0].health=1;g.enemies[0]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};step(&g,(Input){0},1.0f/60);assert(g.state==LOST);
    step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.players[0].health==3);
    /* Stay in an empty lane: all 24 scheduled aliens must resolve and win. */
    start(&g);g.players[0].x=592;g.players[0].y=92;
    for(int i=0;i<2400&&g.state==PLAYING;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==WON&&g.spawned==24&&g.resolved==24&&g.players[0].score==24&&g.players[0].ammo==34);
    step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.players[0].score==0);
    /* Shared enemies resolve once; the shooter owns the reward. */
    start(&g);game_set_players(&g,2);
    g.enemies[0]=(Entity){.x=200,.y=100,.active=1};g.spawned=1;
    g.bullets[0]=(Entity){.x=200,.y=107,.active=1,.owner=1};
    g.bullets[1]=(Entity){.x=200,.y=107,.active=1,.owner=0};
    step(&g,(Input){0},1.0f/60);
    assert(g.resolved==1&&g.players[1].score==3&&g.players[0].score==0);
    assert(!g.enemies[0].active&&g.bullets[1].active);
    /* Independent movement, firing, cooldown and overlapping jets. */
    start(&g);game_set_players(&g,2);g.players[1].x=g.players[0].x;
    Input both[MAX_PLAYERS]={{.x=-1,.fire=1},{.x=1,.fire=1}};
    game_step(&g,both,1.0f/60);
    assert(g.players[0].x<320&&g.players[1].x>320);
    assert(g.players[0].ammo==9&&g.players[1].ammo==9);
    assert(g.players[0].health==3&&g.players[1].health==3);
    assert(g.bullets[0].owner==0&&g.bullets[1].owner==1);
    game_step(&g,both,1.0f/60);assert(g.players[0].ammo==9&&g.players[1].ammo==9);
    g.enemies[0]=(Entity){.x=100,.y=509,.active=1};
    step(&g,(Input){0},1.0f/60);
    assert(g.players[0].score==1&&g.players[1].score==1);
    assert(g.players[0].ammo==10&&g.players[1].ammo==10);
    /* A knocked-out pilot cannot shoot or earn dodge rewards; teammate continues. */
    g.players[0].health=1;
    g.enemies[0]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};
    step(&g,(Input){0},1.0f/60);assert(g.players[0].health==0&&g.state==PLAYING);
    g.players[0].shot_clock=0;
    g.enemies[0]=(Entity){.x=100,.y=509,.active=1};
    game_step(&g,both,1.0f/60);
    assert(g.players[0].ammo==10&&g.players[0].score==1&&g.players[1].score==2);
    g.players[1].health=1;g.players[1].invincible=0;
    g.enemies[0]=(Entity){.x=g.players[1].x,.y=g.players[1].y,.active=1};
    step(&g,(Input){0},1.0f/60);assert(g.state==LOST);
    Input restart[MAX_PLAYERS]={{0},{.start=1}};game_step(&g,restart,1.0f/60);
    assert(g.state==PLAYING&&g.player_count==2&&g.players[0].health==3&&g.players[1].score==0);
    g.players[1].ammo=4;g.players[1].health=2;g.players[1].score=8;
    game_set_players(&g,1);game_set_players(&g,2);
    assert(g.players[1].ammo==4&&g.players[1].health==2&&g.players[1].score==8);
    /* Both pilots complete the unchanged short level. */
    start(&g);game_set_players(&g,2);
    for(int p=0;p<2;++p){g.players[p].x=592;g.players[p].y=92;}
    for(int i=0;i<2400&&g.state==PLAYING;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==WON&&g.resolved==24&&g.players[0].score==24&&g.players[1].score==24);
    puts("PASS: solo and co-op collisions, movement, firing, rewards, damage, waves, restart, hotplug stats");
}
