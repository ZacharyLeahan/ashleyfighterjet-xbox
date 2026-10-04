#include "render.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void color(SDL_Renderer *r, unsigned c) { SDL_SetRenderDrawColor(r,c>>16,(c>>8)&255,c&255,255); }
static void box(SDL_Renderer *r,int x,int y,int w,int h,unsigned c) { SDL_Rect b={x,y,w,h}; color(r,c); SDL_RenderFillRect(r,&b); }
static void ellipse(SDL_Renderer *r,int x,int y,int rx,int ry,unsigned c) {
    color(r,c); for(int dy=-ry;dy<=ry;++dy) {
        int dx=(int)(rx*sqrtf(1.0f-(float)(dy*dy)/(ry*ry)));
        SDL_RenderDrawLine(r,x-dx,y+dy,x+dx,y+dy);
    }
}
/* Tiny original 5x7 font: no external asset or font dependency. */
static const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-!'";
static const unsigned char glyphs[][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
{14,17,16,23,17,17,14},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},
{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
{14,17,17,15,1,1,14},{0,4,4,0,4,4,0},{0,0,0,31,0,0,0},
{4,4,4,4,4,0,4},{4,4,2,0,0,0,0}};
static void text(SDL_Renderer *r,int x,int y,const char *s,int scale,unsigned c) {
    for(;*s;++s,x+=6*scale) { const char *p=strchr(alphabet,*s); if(!p)continue;
        for(int row=0;row<7;++row)for(int col=0;col<5;++col)
            if(glyphs[p-alphabet][row]&(16>>col))box(r,x+col*scale,y+row*scale,scale,scale,c);
    }
}
static void centered(SDL_Renderer *r,int y,const char *s,int scale,unsigned c) { text(r,(640-(int)strlen(s)*6*scale)/2,y,s,scale,c); }
static void jet_box(SDL_Renderer *r,int x,int y,float sx,float sy,int dx,int dy,int w,int h,unsigned c) {
    int ax=x+(int)(dx*sx), bx=x+(int)((dx+w)*sx);
    int ay=y+(int)(dy*sy), by=y+(int)((dy+h)*sy);
    box(r,ax<bx?ax:bx,ay<by?ay:by,abs(bx-ax)+1,abs(by-ay)+1,c);
}
static void jet_ellipse(SDL_Renderer *r,int x,int y,float sx,float sy,int dx,int dy,int rx,int ry,unsigned c) {
    ellipse(r,x+(int)(dx*sx),y+(int)(dy*sy),1+(int)(rx*fabsf(sx)),1+(int)(ry*fabsf(sy)),c);
}
static void jet(SDL_Renderer *r,int x,int y,float t,int player,const Player *p) {
    float sx=1,sy=1;
    if(p->roll_clock>0) {
        float spin=cosf((0.4f-p->roll_clock)/0.3f*6.2831853f);
        if(p->roll_clock<=0.1f)spin=1;
        if(p->roll_x)sx=spin;else sy=spin;
        ellipse(r,x,y,38,38,0x253d66);
    }
    int flame=12+(int)(6*(1+sinf(t*25)));
    jet_ellipse(r,x,y,sx,sy,-8,27,4,flame,0xff9d3f); jet_ellipse(r,x,y,sx,sy,8,27,4,flame,0xff9d3f);
    for(int dy=-10;dy<=26;++dy) { int width=(dy+10)*30/36; jet_box(r,x,y,sx,sy,-width,dy,width*2+1,1,player?0xff799f:0x4f8cff); }
    jet_box(r,x,y,sx,sy,-30,22,5,5,0xffe36e); jet_box(r,x,y,sx,sy,26,22,5,5,0xffe36e);
    jet_ellipse(r,x,y,sx,sy,0,0,9,31,0xbcd4ff); jet_ellipse(r,x,y,sx,sy,0,-7,5,12,0x174b81);
    jet_box(r,x,y,sx,sy,-2,-13,2,8,0xa8e8ff);
}
static void alien(SDL_Renderer *r,int x,int y,int n,float t) {
    static const unsigned colors[]={0xff5e8a,0x2ec5ff,0xffb020,0x3fe08f};
    ellipse(r,x,y-7,14,15,0xa8e8ff); ellipse(r,x,y-9,8,8,0x6fdd6f);
    box(r,x-4,y-12,2,3,0x1a1a2e); box(r,x+3,y-12,2,3,0x1a1a2e);
    ellipse(r,x,y+2,25,10,colors[n%4]);
    for(int i=0;i<5;++i)ellipse(r,x-18+i*9,y+5,2,2,sinf(t*5+i)>0?0xfff8b8:0xbcd4ff);
}
static void blaster(SDL_Renderer *r,const Game *g) {
    const Boss *b=&g->boss; int x=(int)b->x,y=(int)b->y;
    int ry=65+(int)(sinf(b->time*3)*3);
    ellipse(r,x,y,80,80,0x241640);
    /* Three crown points, a purple jelly body, and tracking googly eyes. */
    for(int dy=0;dy<23;++dy) {
        box(r,x-30,y-83+dy,dy/2+1,1,0xffd76e);
        box(r,x+30-dy/2,y-83+dy,dy/2+1,1,0xffd76e);
        box(r,x-dy/2,y-89+dy,dy+1,1,0xffd76e);
    }
    box(r,x-30,y-67,61,12,0xffd76e);
    ellipse(r,x,y,65,ry,b->flash>0?0xffffff:0xb44df0);
    ellipse(r,x-21,y-28,25,19,0xc976f4);
    ellipse(r,x-33,y+22,10,10,0xe0b3ff);ellipse(r,x+36,y+11,7,7,0xe0b3ff);
    int look=(int)((g->players[0].x-b->x)/640*18);
    for(int side=-1;side<=1;side+=2) {
        int ex=x+side*23;
        ellipse(r,ex,y-13,16,19,0xffffff);
        ellipse(r,ex+look,y-10,8,8,0x1a1a2e);
        color(r,0x50236e);
        for(int thick=0;thick<5;++thick)
            SDL_RenderDrawLine(r,ex+side*14,y-40+thick,ex-side*10,y-30+thick);
    }
    color(r,0x1a1a2e);
    for(int dx=-17;dx<=17;++dx) {
        int mouth=y+25+dx*dx/35;
        SDL_RenderDrawLine(r,x+dx,mouth,x+dx,mouth+4);
    }
}
void draw_game(SDL_Renderer *r,const Game *g,float t) {
    color(r,0x0b0b2a); SDL_RenderClear(r);
    for(int i=0;i<100;++i) { int x=24+(i*137)%592; float speed=16+(i%3)*20;
        int y=(int)fmodf(i*79+t*speed,480); box(r,x,y,1+i%2,1+i%2,i%3?0x667aab:0xbcd4ff); }
    if(g->boss.active)blaster(r,g);
    for(int b=0;b<MAX_BOSS_SHOTS;++b)if(g->boss_shots[b].active) {
        const BossShot *shot=&g->boss_shots[b];
        ellipse(r,(int)shot->x,(int)shot->y,9,9,0xb44df0);
        ellipse(r,(int)shot->x,(int)shot->y,5,5,0xff5ecf);
    }
    for(int i=0;i<MAX_ENEMIES;++i)if(g->enemies[i].active)alien(r,(int)g->enemies[i].x,(int)g->enemies[i].y,g->enemies[i].color,t);
    for(int i=0;i<MAX_BULLETS;++i)if(g->bullets[i].active) { Entity b=g->bullets[i]; ellipse(r,(int)b.x,(int)b.y,3,8,b.owner?0xff799f:0xffe36e); }
    for(int i=0;i<g->player_count;++i) {
        const Player *p=&g->players[i];
        if(p->health&&(p->roll_clock>0||p->invincible<=0||(int)(t*12)%2))jet(r,(int)p->x,(int)p->y,t,i,p);
    }
    if(g->state==PLAYING) {
        for(int i=0;i<g->player_count;++i) {
            const Player *p=&g->players[i]; char hud[64];
            snprintf(hud,sizeof(hud),"P%d AMMO %d SCORE %d HEALTH %d",i+1,p->ammo,p->score,p->health);
            centered(r,18+i*20,hud,2,i?0xff799f:0xbcd4ff);
        }
        if(g->boss.active) {
            centered(r,60,"BLASTER BOSS",2,0xe0b3ff);
            box(r,48,79,544,5,0x253058);
            box(r,48,79,544*g->boss.health/BOSS_HEALTH,5,0xb44df0);
        } else {
            box(r,48,62,544,3,0x253058); box(r,48,62,544*g->resolved/LEVEL_ENEMIES,3,0x57e89c);
        }
        centered(r,438,g->boss.active?"DODGE BOSS SHOTS TO EARN BULLETS":"RIGHT STICK TO ROLL AND DODGE",2,0xbcd4ff);
    } else {
        box(r,45,115,550,223,0x111535);
        if(g->state==MENU) {
            centered(r,140,"ASHLEY'S",4,0xffe36e); centered(r,178,"FIGHTER JET",4,0xff9d3f);
            centered(r,232,g->player_count==2?"TWO PILOTS - ONE ADVENTURE!":"DODGE THE SILLY ALIENS!",2,0xbcd4ff);
        } else {
            centered(r,148,g->state==WON?"YOU WIN!":"OOF! YOU GOT BONKED!",g->state==WON?4:3,g->state==WON?0x57e89c:0xff5e8a);
            char score[64];
            if(g->player_count==2) {
                snprintf(score,sizeof(score),"P1 SCORE %d - P2 SCORE %d",g->players[0].score,g->players[1].score);
                centered(r,204,score,2,0xffe36e);
                int a=g->players[0].score,b=g->players[1].score;
                centered(r,238,a==b?"TIED SCORE!":a>b?"P1 TOP SCORE!":"P2 TOP SCORE!",2,0xffffff);
            } else {
                snprintf(score,sizeof(score),"SCORE %d",g->players[0].score); centered(r,206,score,3,0xffe36e);
            }
        }
        centered(r,276,"PRESS START TO FLY",2,0xffffff);
        centered(r,308,"LEFT STICK MOVE - A SHOOT - RIGHT ROLL",2,0xbcd4ff);
    }
    SDL_RenderPresent(r);
}
