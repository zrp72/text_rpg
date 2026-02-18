#include <iostream>
#include <SDL3/SDL.h>

int main(int argc, char** argv){

    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Unable to initialize SDL_video: %s", SDL_GetError());
        SDL_Quit();
    }

    SDL_Window *window = SDL_CreateWindow("TextRPG", 800, 600, SDL_WINDOW_RESIZABLE);

    if(!window){
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if(!renderer){
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_Quit;
    }

    bool running = true;

    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit;

    return 0;
}