/*
Functions to likely use in the future:
TTF_GetStringSize

*/

#include <iostream>
#include <map>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "functions.h"

#define debug(x)  std::cout << x 

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

void
update_paragraph_text(UI_element &paragraph_element, game_data &data){
    char* new_text = data.story[data.story_location].text.data();
    paragraph_element.text[0].TTF_text->text = new_text;
}

int 
launch_game(game_data &data, assets& assets, gui_core& gui){


    for(auto &[tag, obj] : assets.elements){
        for(auto &texture : obj.texture){
            resize_rects(texture.dstrect, gui.window);
        }
        for(auto &text : obj.text){
            resize_text_elements(text, gui.window);
            resize_rects(text.mask, gui.window);
        }
    resize_rects(obj.area, gui.window);
    resize_rects(obj.mask, gui.window);
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
                for(auto &[tag, obj] : assets.elements){
                    for(auto &texture : obj.texture){
                        resize_rects(texture.dstrect, gui.window);
                    }
                    for(auto &text : obj.text){
                        resize_text_elements(text, gui.window);
                        resize_rects(text.mask, gui.window);
                    }
                resize_rects(obj.area, gui.window);
                resize_rects(obj.mask, gui.window);
                }
            }

            if(event.type == SDL_EVENT_MOUSE_MOTION){
                SDL_GetMouseState(&mouse_x, &mouse_y);
                mouse_pos = SDL_Point(mouse_x, mouse_y);
            }

            if(event.type == SDL_EVENT_MOUSE_WHEEL){

                auto clamp_scroll = [](int &max_scroll, int &scroll_y){
                    max_scroll = std::max(0, max_scroll);
                    scroll_y = std::clamp(scroll_y, 0, max_scroll);
                };

                bool scrolled = false;
                for(auto &[tag, obj] : assets.elements){

                    for(auto &text : obj.text){

                        if(SDL_PointInRect(&mouse_pos, &text.mask.SDL_rect)){
                            int max_scroll = text.height - text.mask.SDL_rect.h;
                            text.scroll_y -= event.wheel.y * 25;
                            clamp_scroll(max_scroll, text.scroll_y);

                            if(max_scroll != 0){scrolled = true;}
                            break;
                        }
                    }

                    if(scrolled){break;}else{
                        if(SDL_PointInRect(&mouse_pos, &obj.area.SDL_rect)){
                            int max_scroll = obj.area.SDL_rect.h - obj.mask.SDL_rect.h;
                            obj.scroll_y -= event.wheel.y * 25;
                            clamp_scroll(max_scroll, obj.scroll_y);
                            break;
                        }
                    }
                }
            }

            bool clicked = false;
            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                for(auto& [tag, obj] : assets.elements){
                    for(auto& texture : obj.texture){
                        if(SDL_PointInRect(&mouse_pos, &texture.dstrect.SDL_rect)){
                            if(texture.click_events){
                                texture.click_events(data, assets, gui);
                                clicked  = true;
                                break;
                            }
                        }
                    }
                    if(clicked){break;}
                }
            }
        }

        SDL_SetRenderDrawColor(gui.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gui.renderer);
        
        for (const auto &[tag, obj] : assets.elements){
            if(obj.visible){
                SDL_SetRenderDrawColor(gui.renderer, 255, 192, 203, 255);
                SDL_FRect frect1;
                SDL_RectToFRect(&obj.area.SDL_rect, &frect1);
                SDL_RenderRect(gui.renderer, &frect1);

                SDL_SetRenderClipRect(gui.renderer, &obj.mask.SDL_rect);
                for(auto &t : obj.texture){
                    SDL_FRect srcrect;
                    SDL_FRect dstrect;
                    SDL_RectToFRect(&t.srcrect.SDL_rect, &srcrect);
                    SDL_RectToFRect(&t.dstrect.SDL_rect, &dstrect);
                    dstrect.y -= obj.scroll_y;
                    //add getrectintersection

                    SDL_RenderTexture(gui.renderer, t.SDL_texture, &srcrect, &dstrect);
                }

                for(auto &text : obj.text){
                    SDL_Rect original_mask = text.mask.SDL_rect; 
                    original_mask.y -= obj.scroll_y;
                    SDL_Rect mask;
                    
                    
                    if(SDL_GetRectIntersection(&original_mask, &obj.mask.SDL_rect, &mask)){
                        SDL_SetRenderClipRect(gui.renderer, &mask);
                        SDL_SetRenderDrawColor(gui.renderer, 255, 255, 255, 255);
                        SDL_FRect frect;
                        SDL_RectToFRect(&mask, &frect);
                        SDL_RenderRect(gui.renderer, &frect);

                        TTF_DrawRendererText(text.TTF_text, text.x, text.y - text.scroll_y - obj.scroll_y);
                    }
                }
                SDL_SetRenderClipRect(gui.renderer, NULL);
            }
        }
        
        SDL_RenderPresent(gui.renderer);
    }
    //gui_core destructor handles rest of SDL and TTF destructions

    return 0;
}