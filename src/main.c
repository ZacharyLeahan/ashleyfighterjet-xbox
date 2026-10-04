#include <SDL.h>
#include <hal/video.h>
#include <hal/debug.h>
#include <windows.h>
int main(void) {
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        debugPrint("SDL init: %s\n", SDL_GetError()); for (;;) Sleep(1000);
    }
    SDL_Window *w = SDL_CreateWindow("Ashley Fighter Jet", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer *r = w ? SDL_CreateRenderer(w, -1, 0) : NULL;
    if (!r) { debugPrint("Renderer: %s\n", SDL_GetError()); for (;;) Sleep(1000); }
    for (;;) {
        SDL_Event e; while (SDL_PollEvent(&e)) {}
        SDL_SetRenderDrawColor(r, 11, 11, 42, 255); SDL_RenderClear(r);
        SDL_SetRenderDrawColor(r, 79, 140, 255, 255);
        SDL_Rect jet = {300, 340, 40, 64}; SDL_RenderFillRect(r, &jet);
        SDL_RenderPresent(r); SDL_Delay(16);
    }
    return 0;
}
