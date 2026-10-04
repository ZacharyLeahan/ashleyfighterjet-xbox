#ifndef GAME_H
#define GAME_H
#define GAME_W 640
#define GAME_HGT 480
#define MAX_ENEMIES 12
#define MAX_BULLETS 24
#define LEVEL_ENEMIES 24
typedef enum { MENU, PLAYING, WON, LOST } GameState;
typedef struct { float x, y; int active, color; } Entity;
typedef struct { float x, y; int fire, start; } Input;
typedef struct {
    GameState state;
    float x, y, elapsed, spawn_clock, shot_clock, invincible;
    int health, score, ammo, spawned, resolved, start_held;
    Entity enemies[MAX_ENEMIES], bullets[MAX_BULLETS];
} Game;
void game_init(Game *g);
void game_step(Game *g, Input input, float dt);
int circles_hit(float ax, float ay, float ar, float bx, float by, float br);
#endif
