/*
Functions to likely use in the future:
TTF_GetStringSize

*/

#include <iostream>
#include <map>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "functions.h"

#define debug(x)  std::cout << x 

void
resize_text_elements(UI_text_element &t, SDL_Window* window){
    printf("T=%x\n", t);
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

    TTF_SetTextWrapWidth(t.text, window_width * t.relative_wrap_width);

    int text_width;
    int text_height;
    TTF_GetTextSize(t.text, &text_width, &text_height);
    t.width = text_width;
    t.height = text_height;

    t.x = (window_width - t.width) * t.relative_x;
    t.y = window_height * t.relative_y;
}

void
resize_rects(relative_rect &rect, SDL_Window* window){
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

    rect.rect.w = window_width * rect.relw;
    rect.rect.h = window_height * rect.relh;
    rect.rect.x = (window_width - rect.rect.w) * rect.relx;
    rect.rect.y = window_height * rect.rely;
}

UI_text_element 
create_paragraph_text_object(TTF_Font* font, TTF_TextEngine* text_engine){
    UI_text_element text_element;
    TTF_Text* text = TTF_CreateText(text_engine, font, "TestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestPINEAPPLEBANANA", 0);
    if (!text) {
        debug("Text failed to create: " << SDL_GetError() << "\n");
    }
    TTF_SetTextColor(text, 255, 255, 255, 255);

    text_element.relative_wrap_width = .7;
    text_element.relative_x = .5;
    text_element.relative_y = .1;
    text_element.text = text;

    return text_element;
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
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if(!renderer){
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_Quit();
    }

    TTF_Font *font = TTF_OpenFont("ttf files/Roboto-Light.ttf", 24);
    if (!font) {
        std::cout << "Font failed to load: " << SDL_GetError() << "\n";
    }
    TTF_SetFontWrapAlignment(font, TTF_HORIZONTAL_ALIGN_CENTER);

    TTF_TextEngine* text_engine = TTF_CreateRendererTextEngine(renderer);
    const char* story_text = data.story[data.story_location].text.c_str();
    UI_text_element paragraph_text_object = create_paragraph_text_object(font, text_engine);
    resize_text_elements(paragraph_text_object, window);
    

    relative_rect paragraph_viewport(.5, .1, .7, .5);
    resize_rects(paragraph_viewport, window);

    bool running = true;
    int scroll_y = 0;
    while(running){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if(event.type == SDL_EVENT_WINDOW_RESIZED){
                resize_text_elements(paragraph_text_object, window);
                resize_rects(paragraph_viewport, window);
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                scroll_y -= event.wheel.y * 20;
                int maxScroll = paragraph_text_object.height - paragraph_viewport.rect.h;
                if (maxScroll < 0){
                    maxScroll = 0;
                }
                if (scroll_y < 0){
                    scroll_y = 0;
                }
                if (scroll_y > maxScroll){
                    scroll_y = maxScroll;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderClipRect(renderer, &paragraph_viewport.rect);
        TTF_DrawRendererText(paragraph_text_object.text, paragraph_text_object.x, paragraph_text_object.y - scroll_y);
        SDL_SetRenderClipRect(renderer, NULL);
        
        SDL_RenderPresent(renderer);
    }

    TTF_DestroyText(paragraph_text_object.text);
    TTF_DestroySurfaceTextEngine(text_engine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}