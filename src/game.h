#ifndef GAME_H
#define GAME_H
#define GAME_W 640
#define GAME_HGT 480
#define MAX_PLAYERS 2
#define MAX_ENEMIES 12
#define MAX_BULLETS 48
#define LEVEL_ENEMIES 24
typedef enum { MENU, PLAYING, WON, LOST } GameState;
typedef struct { float x, y; int active, color, owner; } Entity;
typedef struct { float x, y; int fire, start; } Input;
typedef struct {
    float x, y, shot_clock, invincible;
    int health, score, ammo;
} Player;
typedef struct {
    GameState state;
    float elapsed, spawn_clock;
    int player_count, spawned, resolved, start_held;
    Player players[MAX_PLAYERS];
    Entity enemies[MAX_ENEMIES], bullets[MAX_BULLETS];
} Game;
void game_init(Game *g);
void game_set_players(Game *g, int count);
void game_step(Game *g, const Input inputs[MAX_PLAYERS], float dt);
int circles_hit(float ax, float ay, float ar, float bx, float by, float br);
#endif
