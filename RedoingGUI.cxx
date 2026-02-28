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

SDL_FRect
convert_rect_to_frect(SDL_Rect rect){
    SDL_FRect frect(rect.x, rect. y, rect.w, rect.h);
    return frect;
}

void
resize_text_elements(relative_text &t, SDL_Window* window){
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

    SDL_Rect bounds(t.x, t.y, t.width, t.height);
    t.bounds = bounds;
}

void
resize_rects(relative_rect &rect, SDL_Window* window){
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

    rect.rect.w = window_width * rect.relative_w;
    rect.rect.h = window_height * rect.relative_h;
    rect.rect.x = (window_width - rect.rect.w) * rect.relative_x;
    rect.rect.y = window_height * rect.relative_y;
}

relative_text 
create_paragraph_text_object(TTF_Font* font, TTF_TextEngine* text_engine){
    relative_text text_element;
    TTF_Text* text = TTF_CreateText(text_engine, font, "TestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaTestAPPLEBANANAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaav", 0);
    if (!text) {
        debug("Text failed to create: " << SDL_GetError() << "\n");
    }
    TTF_SetTextColor(text, 255, 255, 255, 255);

    text_element.relative_wrap_width = .7;
    text_element.relative_x = .5;
    text_element.relative_y = .1;
    text_element.text = text;

    relative_rect viewport(.5, .1, .7, .5);
    text_element.mask = viewport;

    return text_element;
}

UI_element
create_paragraph_element(TTF_Font *font, TTF_TextEngine *text_engine){
    UI_element element;
    element.rel_text.push_back(create_paragraph_text_object(font, text_engine));
    
    element.visible = true;
    return element;
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
    
    std::map<std::string, UI_element> UI_elements;
    UI_elements["Story_display"] = (create_paragraph_element(font, text_engine));

    for(auto &[tag, obj] : UI_elements){
        for(auto &rel_text : obj.rel_text){
            resize_text_elements(rel_text, window);
            resize_rects(rel_text.mask, window);
        }
        for(auto &rel_tex : obj.texture){
            resize_rects(rel_tex.srcrect, window);
            resize_rects(rel_tex.dstrect, window);
        }
    }

    bool running = true;
    float mouse_x;
    float mouse_y;
    SDL_Point mouse_pos;

    while(running){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if(event.type == SDL_EVENT_WINDOW_RESIZED){
                for(auto &[tag, obj] : UI_elements){
                    for(auto &rel_text : obj.rel_text){
                        resize_text_elements(rel_text, window);
                        resize_rects(rel_text.mask, window);
                    }
                    for(auto &rel_tex : obj.texture){
                        resize_rects(rel_tex.srcrect, window);
                        resize_rects(rel_tex.dstrect, window);
                    }
                }
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                SDL_GetMouseState(&mouse_x, &mouse_y);
                mouse_pos = SDL_Point(mouse_x, mouse_y);

                for(auto &[tag, obj] : UI_elements){
                    for(auto &text : obj.rel_text){
                        if(SDL_PointInRect(&mouse_pos, &text.mask.rect)){
                            int max_scroll = text.height - text.mask.rect.h;
                            text.scroll_y -= event.wheel.y * 25;
                            if (max_scroll < 0){
                                max_scroll = 0;
                            }
                            if (text.scroll_y < 0){
                                text.scroll_y = 0;
                            }
                            if (text.scroll_y > max_scroll){
                                text.scroll_y = max_scroll;
                            }
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        for (const auto &[tag, obj] : UI_elements){
            if(obj.visible){
                for(auto &t : obj.texture){
                    SDL_FRect srcrect = convert_rect_to_frect(t.srcrect.rect);
                    SDL_FRect dstrect = convert_rect_to_frect(t.dstrect.rect);
                    SDL_RenderTexture(renderer, t.texture, &srcrect, &dstrect);
                }
                for(auto &rel_text : obj.rel_text){
                    SDL_SetRenderClipRect(renderer, &rel_text.mask.rect);
                    TTF_DrawRendererText(rel_text.text, rel_text.x, rel_text.y - rel_text.scroll_y);
                    SDL_SetRenderClipRect(renderer, NULL);
                }
            }
        }
        
        SDL_RenderPresent(renderer);
    }

    for(auto &[tag, obj] : UI_elements){
        for(auto &t : obj.texture){
            SDL_DestroyTexture(t.texture);
        }
        for(auto &rel_text : obj.rel_text){
            TTF_DestroyText(rel_text.text);
        }
    }
    TTF_DestroySurfaceTextEngine(text_engine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}