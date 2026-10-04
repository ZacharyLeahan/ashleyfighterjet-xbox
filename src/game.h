#ifndef GAME_H
#define GAME_H
#define GAME_W 640
#define GAME_HGT 480
#define MAX_PLAYERS 2
#define MAX_ENEMIES 12
#define MAX_BULLETS 48
#define LEVEL_ENEMIES 24
#define BOSS_HEALTH 16
#define MAX_BOSS_SHOTS 32
typedef enum { MENU, PLAYING, DYING, WON, LOST } GameState;
typedef enum {
    SOUND_SHOOT, SOUND_POP, SOUND_EMPTY, SOUND_ALARM, SOUND_BOSS_SHOOT,
    SOUND_BOSS_HIT, SOUND_BOSS_DIE, SOUND_WIN, SOUND_CRASH, SOUND_COUNT
} SoundEvent;
#define MAX_SOUND_EVENTS 32
typedef struct { float x, y; int active, color, owner; } Entity;
typedef struct { float x, y; int fire, start; float roll_x, roll_y; } Input;
typedef struct {
    float x, y, shot_clock, invincible;
    float roll_clock, roll_x, roll_y;
    int roll_held;
    int health, score, ammo;
} Player;
typedef struct { float x,y,vx,vy; int active; } BossShot;
typedef struct {
    float x,y,time,flash,fire_clock,death_time,fall_speed;
    int active,entering,health,volleys;
} Boss;
typedef struct {
    GameState state;
    SoundEvent sounds[MAX_SOUND_EVENTS];
    int sound_count;
    float elapsed, spawn_clock;
    int player_count, spawned, resolved, start_held;
    Player players[MAX_PLAYERS];
    Entity enemies[MAX_ENEMIES], bullets[MAX_BULLETS];
    Boss boss;
    BossShot boss_shots[MAX_BOSS_SHOTS];
} Game;
void game_init(Game *g);
void game_set_players(Game *g, int count);
void game_step(Game *g, const Input inputs[MAX_PLAYERS], float dt);
int circles_hit(float ax, float ay, float ar, float bx, float by, float br);
#endif
