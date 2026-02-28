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

relative_rect
determine_element_area(UI_element &element){
    float min_x = 1;
    float min_y = 1;
    float top_w = 0;
    float top_h = 0;

    auto resize = [&](float x, float y, float w, float h){
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        top_w = std::max(top_w, x + w);
        top_h = std::max(top_h, y + h);
    };

    for(auto &texture : element.texture){
        auto t = texture.dstrect;
        resize(t.relative_x, t.relative_y, t.relative_w, t.relative_h);
    }
    for(auto &text : element.text){
        resize(text.relative_x, text.relative_y, text.relative_wrap_width, text.mask.relative_h);
    }

    relative_rect area(min_x, min_y, top_w - min_x, top_h - min_y);
    return area;
}

void
resize_text_elements(relative_text &t, SDL_Window* window){
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

    TTF_SetTextWrapWidth(t.TTF_text, window_width * t.relative_wrap_width);

    int text_width;
    int text_height;
    TTF_GetTextSize(t.TTF_text, &text_width, &text_height);
    t.width = text_width;
    t.height = text_height;


    t.x = window_width * t.relative_x;
    t.y = window_height * t.relative_y;

    SDL_Rect bounds(t.x, t.y, t.width, t.height);
    t.bounds = bounds;
}

void
resize_rects(relative_rect &rect, SDL_Window* window){
    int window_width;
    int window_height;
    SDL_GetWindowSizeInPixels(window, &window_width, &window_height);


    rect.SDL_rect.x = window_width * rect.relative_x;
    rect.SDL_rect.y = window_height * rect.relative_y;

    rect.SDL_rect.w = window_width * rect.relative_w;
    rect.SDL_rect.h = window_height * rect.relative_h;
}

relative_text 
create_paragraph_text(TTF_Font* font, TTF_TextEngine* text_engine, game_data &data){
    relative_text text_element;
    const char* story_text = data.story[data.story_location].text.c_str();
    TTF_Text* text = TTF_CreateText(text_engine, font, story_text, 0);
    if (!text) {
        debug("Text failed to create: " << SDL_GetError() << "\n");
    }
    TTF_SetTextColor(text, 255, 255, 255, 255);

    text_element.relative_wrap_width = .7;
    text_element.relative_x = .15;
    text_element.relative_y = .1;
    text_element.TTF_text = text;

    relative_rect viewport(.15, .1, .7, .5);
    text_element.mask = viewport;

    return text_element;
}

UI_element
create_paragraph_element(TTF_Font *font, TTF_TextEngine *text_engine, game_data &data){
    UI_element element;
    element.text.push_back(create_paragraph_text(font, text_engine, data));
    element.area = determine_element_area(element);
    debug(element.area.SDL_rect.x << "x, " << element.area.SDL_rect.y << "y, " << element.area.SDL_rect.w << "w, " << element.area.SDL_rect.h << "h\n");
    
    element.visible = true;
    return element;
}

void
update_paragraph_text(UI_element &paragraph_element, game_data &data){
    char* new_text = data.story[data.story_location].text.data();
    paragraph_element.text[0].TTF_text->text = new_text;
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
    UI_elements["Story_display"] = (create_paragraph_element(font, text_engine, data));

    for(auto &[tag, obj] : UI_elements){
        for(auto &text : obj.text){
            resize_text_elements(text, window);
            resize_rects(text.mask, window);
        }
        for(auto &texture : obj.texture){
            resize_rects(texture.srcrect, window);
            resize_rects(texture.dstrect, window);
        }
        resize_rects(obj.area, window);
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
                    for(auto &text : obj.text){
                        resize_text_elements(text, window);
                        resize_rects(text.mask, window);
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
                    for(auto &text : obj.text){
                        if(SDL_PointInRect(&mouse_pos, &text.mask.SDL_rect)){
                            int max_scroll = text.height - text.mask.SDL_rect.h;
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
                        //if(SDL_PointInRect(&mouse_pos, obj.area))
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        for (const auto &[tag, obj] : UI_elements){
            if(obj.visible){
                for(auto &t : obj.texture){
                    SDL_FRect srcrect = convert_rect_to_frect(t.srcrect.SDL_rect);
                    SDL_FRect dstrect = convert_rect_to_frect(t.dstrect.SDL_rect);
                    SDL_RenderTexture(renderer, t.texture, &srcrect, &dstrect);
                }
                for(auto &text : obj.text){
                    SDL_SetRenderClipRect(renderer, &text.mask.SDL_rect);
                    TTF_DrawRendererText(text.TTF_text, text.x, text.y - text.scroll_y);
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
        for(auto &text : obj.text){
            TTF_DestroyText(text.TTF_text);
        }
    }
    TTF_DestroySurfaceTextEngine(text_engine);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}