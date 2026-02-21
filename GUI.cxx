/*#include <iostream>
#include <map>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "functions.h"

#define debug(x)  std::cout << x 

bool
create_story_display(SDL_Window *window, SDL_Renderer *renderer, auto &elements, auto font){
    int screenW;
    int screenH;
    SDL_GetWindowSizeInPixels(window, &screenW, &screenH);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, "Text RPG", 0, white, screenW * .7);

    if (!surface){
        SDL_Log("Failed to create text surface: %s", SDL_GetError());
        return 0;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        SDL_Log("Failed to create story display texture: %s", SDL_GetError());
        return 0;
    }

    relative_rect rect(.5, .3, texture->w, texture->h);
    elements["story_display"] = element(texture, rect);
    return 1;
}

int 
present_game(game_data &data){

    if(!SDL_Init(SDL_INIT_VIDEO)){
        SDL_Log("Failed to initialize SDL_video: %s", SDL_GetError());
        SDL_Quit();
    }
    
    if(!TTF_Init()){
        SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
        SDL_Quit();
    }

    SDL_Window *window = SDL_CreateWindow("TextRPG", 800, 600, SDL_WINDOW_RESIZABLE);
    if(!window){
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if(!renderer){
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_Quit();
    }

    TTF_Font *roboto_light = TTF_OpenFont("ttf files/Roboto-Light.ttf", 24);
    if (!roboto_light){
        SDL_Log("Failed to load font: %s", SDL_GetError());
        SDL_Quit();
    }

    std::map<std::string, element> elements;

    if(!create_story_display(window, renderer, elements, roboto_light)){
        SDL_Log("Failed to create story display: %s", SDL_GetError());
        SDL_Quit();
    };

    
    SDL_FRect frect;

    bool running = true;

    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if(event.type == SDL_EVENT_WINDOW_RESIZED){
                int screenW;
                int screenH;
                SDL_GetWindowSizeInPixels(window, &screenW, &screenH);
                for(auto &[string, element] : elements){
                    element.rect.resize_rect(screenW, screenH);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        relative_rect &rel = elements["story_display"].rect;
        frect = {rel.x, rel.y, rel.w, rel.h};
        SDL_RenderTexture(renderer, elements["story_display"].texture, NULL, &frect);

        SDL_RenderPresent(renderer);
    }

    for (auto &[key, element] : elements) {
        if (element.texture) {
            SDL_DestroyTexture(element.texture);
            element.texture = nullptr;
        }
    }
    elements.clear();
    TTF_CloseFont(roboto_light);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}*/