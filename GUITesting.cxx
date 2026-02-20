#include <iostream>
#include <map>
#include <SDL3/SDL.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <filesystem>

#define debug(x)  std::cout << x 

class element {
    public:
    SDL_Texture *texture;
    SDL_Rect rect;
    bool visible = false;

    element():visible(false){};
    element(SDL_Texture *t, SDL_Rect r) : texture(t), rect(r){};

    void resize_rect(SDL_Surface *window_surface){

    }
};

bool
create_story_display(SDL_Renderer *renderer, auto &elements, auto font){
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, "Test", 0, white, 600);

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

    SDL_Rect rect = {100, 100, 100, 100};
    elements["story_display"] = element(texture, rect);
    return 1;
}

int main(int argc, char** argv){
    debug("Original directory: " << std::filesystem::current_path() << '\n');
    std::string base_path = SDL_GetBasePath();
    std::filesystem::current_path(base_path);
    debug("Original directory: " << std::filesystem::current_path() << '\n');
    //Check if this is fine

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
    if(!create_story_display(renderer, elements, roboto_light)){
        SDL_Log("Failed to create story display: %s", SDL_GetError());
        SDL_Quit();
    };


    bool running = true;

    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            /*if(event.type == SDL_EVENT_WINDOW_RESIZED){
                SDL_Surface *window_surface = SDL_GetWindowSurface(window);
                for(auto &[string, element] : elements){
                    element.resize_rect(window_surface);
                }
            }*/
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderTexture(renderer, elements["story_display"].texture, NULL, NULL);     

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
    debug(__LINE__ << '\n');
    SDL_Quit();
    debug(__LINE__ << '\n');

    return 0;
}