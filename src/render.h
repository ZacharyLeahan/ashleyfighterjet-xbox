#ifndef RENDER_H
#define RENDER_H
#include <SDL.h>
#include "game.h"
void draw_game(SDL_Renderer *r, const Game *g, float time);
#endif
