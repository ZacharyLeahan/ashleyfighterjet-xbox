#include "game.h"
#include <assert.h>
#include <stdio.h>
static int heard(const Game *g,SoundEvent sound) {
    for(int i=0;i<g->sound_count;++i)if(g->sounds[i]==sound)return 1;
    return 0;
}
static void step(Game *g, Input in, float dt) { Input inputs[MAX_PLAYERS]={in,{0}};game_step(g,inputs,dt); }
static void start(Game *g) { game_init(g);step(g,(Input){.start=1},1.0f/60);step(g,(Input){0},1.0f/60); }
int main(void) {
    assert(circles_hit(0,0,5,10,0,5));assert(!circles_hit(0,0,5,10.01f,0,5));
    Game g;start(&g);assert(g.state==PLAYING&&g.players[0].ammo==10&&g.players[0].health==3);
    for(int i=0;i<1000;++i)step(&g,(Input){.x=1,.y=1},1.0f/60);
    assert(g.players[0].x<=592&&g.players[0].y<=400);
    start(&g);step(&g,(Input){.fire=1},1.0f/60);assert(g.players[0].ammo==9&&heard(&g,SOUND_SHOOT));
    step(&g,(Input){.fire=1},1.0f/60);assert(g.players[0].ammo==9&&!heard(&g,SOUND_SHOOT));
    start(&g);g.players[0].ammo=0;
    step(&g,(Input){.fire=1},1.0f/60);assert(heard(&g,SOUND_EMPTY));
    step(&g,(Input){.fire=1},1.0f/60);assert(g.sound_count==0);
    start(&g);g.enemies[0]=(Entity){.x=100,.y=509,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].score==1&&g.players[0].ammo==11&&g.resolved==1);
    start(&g);g.enemies[0]=(Entity){.x=200,.y=100,.active=1};g.bullets[0]=(Entity){.x=200,.y=107,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].score==3&&g.resolved==1&&!g.bullets[0].active&&heard(&g,SOUND_POP));
    start(&g);g.enemies[0]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};g.spawned=1;
    step(&g,(Input){0},1.0f/60);assert(g.players[0].health==2&&g.players[0].invincible>0&&heard(&g,SOUND_CRASH));
    g.enemies[1]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};step(&g,(Input){0},1.0f/60);assert(g.players[0].health==2);
    start(&g);g.players[0].health=1;g.enemies[0]=(Entity){.x=g.players[0].x,.y=g.players[0].y,.active=1};step(&g,(Input){0},1.0f/60);assert(g.state==LOST);
    step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.players[0].health==3);
    /* Stay in an empty lane: all 24 scheduled aliens must resolve and win. */
    start(&g);g.players[0].x=592;g.players[0].y=92;
    for(int i=0;i<2400&&g.state==PLAYING&&!g.boss.active;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==PLAYING&&g.boss.active&&g.boss.health==16&&g.spawned==24&&g.resolved==24&&g.players[0].score==24&&g.players[0].ammo==34);
    g.boss.health=0;step(&g,(Input){0},1.0f/60);assert(g.state==DYING);
    for(int i=0;i<180&&g.state==DYING;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==WON);
    step(&g,(Input){.start=1},1.0f/60);assert(g.state==PLAYING&&g.players[0].score==0);
    /* Four directional dodges work without ammo and preserve bounds. */
    for(int direction=0;direction<4;++direction) {
        start(&g);g.players[0].ammo=0;
        float x=g.players[0].x,y=g.players[0].y;
        Input roll={.roll_x=direction<2?(direction?1:-1):0,
                    .roll_y=direction>=2?(direction==2?-1:1):0};
        g.enemies[0]=(Entity){.x=x,.y=y,.active=1};
        step(&g,roll,1.0f/60);
        assert(g.players[0].health==3&&g.players[0].roll_clock>0);
        for(int i=0;i<30;++i)step(&g,roll,1.0f/60);
        assert(g.players[0].roll_clock==0&&g.players[0].ammo==0);
        assert(direction==0?g.players[0].x<x:direction==1?g.players[0].x>x:
               direction==2?g.players[0].y<y:g.players[0].y>y);
        /* Holding cannot auto-roll; releasing allows another immediately. */
        step(&g,(Input){0},1.0f/60);step(&g,roll,1.0f/60);
        assert(g.players[0].roll_clock>0);
    }
    start(&g);g.players[0].x=592;g.players[0].y=92;
    for(int i=0;i<30;++i)step(&g,(Input){.roll_y=-1},1.0f/60);
    assert(g.players[0].y==92);
    step(&g,(Input){0},1.0f/60);
    for(int i=0;i<30;++i)step(&g,(Input){.roll_x=1},1.0f/60);
    assert(g.players[0].x==592);
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
    for(int i=0;i<2400&&g.state==PLAYING&&!g.boss.active;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==PLAYING&&g.boss.active&&g.resolved==24&&g.players[0].score==24&&g.players[1].score==24);
    /* Boss entry refills living pilots, and does not reset their health/score. */
    start(&g);game_set_players(&g,2);g.resolved=LEVEL_ENEMIES;g.spawned=LEVEL_ENEMIES;
    g.players[0].ammo=0;g.players[1].ammo=30;g.players[0].score=7;
    step(&g,(Input){0},1.0f/60);
    assert(heard(&g,SOUND_ALARM));
    assert(g.boss.active&&g.boss.entering&&g.players[0].ammo==24&&g.players[1].ammo==30&&g.players[0].score==7);
    for(int i=0;i<240&&g.boss.entering;++i)step(&g,(Input){0},1.0f/60);
    assert(!g.boss.entering&&g.boss.volleys==0);
    /* Low health changes one aimed bolt into a three-shot spread. */
    g.boss.health=3;g.boss.fire_clock=0;
    step(&g,(Input){0},1.0f/60);
    int shots=0;for(int i=0;i<MAX_BOSS_SHOTS;++i)shots+=g.boss_shots[i].active;
    assert(shots==3&&g.boss.volleys==1&&heard(&g,SOUND_BOSS_SHOOT));
    /* Dodging replenishes ammo; a hit damages just one pilot. */
    for(int i=0;i<MAX_BOSS_SHOTS;++i)g.boss_shots[i].active=0;
    g.boss_shots[0]=(BossShot){.x=100,.y=509,.vy=310,.active=1};
    step(&g,(Input){0},1.0f/60);assert(g.players[0].ammo==25&&g.players[1].ammo==31);
    g.boss_shots[0]=(BossShot){.x=g.players[1].x,.y=g.players[1].y,.active=1};
    step(&g,(Input){0},1.0f/60);assert(g.players[0].health==3&&g.players[1].health==2);
    g.boss_shots[0]=(BossShot){.x=g.players[0].x,.y=g.players[0].y,.active=1};
    step(&g,(Input){.roll_x=1},1.0f/60);assert(g.players[0].health==3);
    /* Player-owned hits defeat Blaster and award the final hitter the bonus. */
    g.boss.health=1;g.boss_shots[0].active=0;
    g.bullets[0]=(Entity){.x=g.boss.x,.y=g.boss.y+20,.active=1,.owner=1};
    step(&g,(Input){0},1.0f/60);
    assert(g.state==DYING&&g.boss.active&&g.players[1].score==11);
    assert(heard(&g,SOUND_BOSS_HIT)&&heard(&g,SOUND_BOSS_DIE)&&!heard(&g,SOUND_WIN));
    float boss_y=g.boss.y;int volleys=g.boss.volleys,health=g.players[0].health;
    step(&g,(Input){.start=1,.fire=1},1.0f/60);
    assert(g.state==DYING&&g.boss.y>boss_y&&g.boss.volleys==volleys&&g.players[0].health==health);
    for(int i=0;i<180&&g.state==DYING;++i)step(&g,(Input){0},1.0f/60);
    assert(g.state==WON&&!g.boss.active&&heard(&g,SOUND_WIN));
    step(&g,(Input){0},1.0f/60);assert(g.sound_count==0);
    step(&g,(Input){.start=1},1.0f/60);
    assert(g.state==PLAYING&&!g.boss.active&&g.boss.health==0);
    /* Boss projectiles can end a run; restarting clears every projectile. */
    g.resolved=g.spawned=LEVEL_ENEMIES;game_set_players(&g,1);
    step(&g,(Input){0},1.0f/60);g.players[0].health=1;
    g.boss_shots[0]=(BossShot){.x=g.players[0].x,.y=g.players[0].y,.active=1};
    step(&g,(Input){0},1.0f/60);assert(g.state==LOST);
    step(&g,(Input){.start=1},1.0f/60);assert(!g.boss_shots[0].active&&!g.boss.active);
    puts("PASS: solo and co-op collisions, movement, firing, rewards, damage, waves, restart, hotplug stats, rolls, Blaster boss");
}
