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
relative_text::resize(gui_core& gui, assets& assets){
	gui_core::window_size ws;
	SDL_GetWindowSizeInPixels(gui.window, &ws.width, &ws.height);

	this->dstrect.SDL_rect.x = ws.width * this->dstrect.relative_x;
	this->dstrect.SDL_rect.y = ws.height * this->dstrect.relative_y;
	this->dstrect.SDL_rect.w = ws.width * this->dstrect.relative_w;

	class word {
		public:
		std::vector<glyph> characters;
		int length = 0;

		word(){}
	};

	class line {
		public:
		std::vector<word> text;
		int length = 0;

		line(){}
	};

	int highest;
	int lowest;
	std::vector<word> words;
	word current_word;
	for(auto& glyph : this->text){
		if(glyph.unicode != U' '){
			auto& glyph_data = assets.glyphs.at(glyph);
			highest = std::max(highest, glyph_data.max_x);
			lowest = std::min(lowest, glyph_data.min_x);
			current_word.characters.push_back(glyph);
			current_word.length += glyph_data.max_x;
		}else{
			words.push_back(current_word);
			current_word = word();
		}
	}
	if(!current_word.characters.empty()){
		words.push_back(current_word);
	}

	int font_height = highest - lowest;

	std::vector<line> lines;
	line current_line;
	for(auto& word : words){
		if(current_line.length + word.length > dstrect.SDL_rect.w){
			lines.push_back(current_line);
			current_line = line();
		}else{
			current_line.text.push_back(word);
			current_line.length += word.length;
		}
	}
	if(!current_line.text.empty()){
		lines.push_back(current_line);
	}

	int height = lines.size() * font_height;
	SDL_Surface* text_texture = SDL_CreateSurface(this->dstrect.SDL_rect.w, height,
		SDL_PIXELFORMAT_ARGB8888);

	int pen_x = 0;
	int pen_y = 0;
	for(line& line : lines){
		for(word& word : line.text){
			for(glyph& glyph : word.characters){

			}
		}
	}

	int max_scroll = this->dstrect.SDL_rect.h - this->mask.SDL_rect.h;
	max_scroll = std::max(0, max_scroll);
	this->scroll_y = std::clamp(this->scroll_y, 0, max_scroll);
}

void
resize_rect(relative_rect &rect, SDL_Window* window){
	gui_core::window_size ws;
	SDL_GetWindowSizeInPixels(window, &ws.width, &ws.height);

	rect.SDL_rect.x = ws.width * rect.relative_x;
	rect.SDL_rect.y = ws.height * rect.relative_y;

	rect.SDL_rect.w = ws.width * rect.relative_w;
	rect.SDL_rect.h = ws.height * rect.relative_h;
}

void
resize_element(UI_element &obj, gui_core& gui, assets& assets){
	resize_rect(obj.area, gui.window);
	resize_rect(obj.mask, gui.window);
	for (auto &texture : obj.texture) {
		resize_rect(texture.dstrect, gui.window);
	}
	for (auto &text : obj.text) {
		resize_rect(text.mask, gui.window);
		text.resize(gui, assets);
	}

	int max_scroll = obj.area.SDL_rect.h - obj.mask.SDL_rect.h;
	max_scroll = std::max(0, max_scroll);
	obj.scroll_y = std::clamp(obj.scroll_y, 0, max_scroll);
}

void
render_debug_hitbox(gui_core& gui, SDL_Rect area, int r, int g, int b, int a){
	SDL_SetRenderDrawColor(gui.renderer, r, g, b, a);
	SDL_FRect frect1;
	SDL_RectToFRect(&area, &frect1);
	SDL_RenderRect(gui.renderer, &frect1);
}

void
render_element(const UI_element& obj, gui_core& gui, assets& assets){

	render_debug_hitbox(gui, obj.area.SDL_rect, 255, 192, 203, 255);

	SDL_SetRenderClipRect(gui.renderer, &obj.mask.SDL_rect);

	for(auto &t : obj.texture){
		SDL_FRect srcrect;
		SDL_FRect dstrect;
		SDL_RectToFRect(&t.srcrect, &srcrect);
		SDL_RectToFRect(&t.dstrect.SDL_rect, &dstrect);
		dstrect.y -= obj.scroll_y;

		SDL_Texture* texture = assets.textures[t.texture_ID];
		SDL_RenderTexture(gui.renderer, texture, &srcrect, &dstrect);
	}

	for(auto &t : obj.text){

		SDL_Rect text_mask = t.mask.SDL_rect;
		text_mask.y -= obj.scroll_y;

		SDL_Rect mask;
		if(SDL_GetRectIntersection(&text_mask, &obj.mask.SDL_rect, &mask)){
			SDL_SetRenderClipRect(gui.renderer, &mask);

			render_debug_hitbox(gui, mask, 255, 255, 255, 255);

			SDL_FRect dstrect;
			SDL_RectToFRect(&t.dstrect.SDL_rect, &dstrect);
			SDL_RenderTexture(gui.renderer, t.texture, NULL, &dstrect);
		}
	}
}

int
launch_game(game_data &data, assets& assets, gui_core& gui){

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
					resize_element(obj, gui, assets);
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
							int max_scroll = text.dstrect.SDL_rect.h - text.mask.SDL_rect.h;
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

			if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
				bool clicked = false;

				for(auto& [tag, obj] : assets.elements){
					for(auto& t : obj.texture){

						SDL_Rect visible_dstrect(t.dstrect.SDL_rect);
						visible_dstrect.y -= obj.scroll_y;

						if(SDL_PointInRect(&mouse_pos, &visible_dstrect)){
							if(t.click_events){
								t.click_events(data, assets, gui);
								clicked = true;
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
				render_element(obj, gui, assets);
			}
		}

		SDL_RenderTexture(gui.renderer, assets.textures["glyph_atlas1"], NULL, NULL);

		SDL_RenderPresent(gui.renderer);
	}

	return 0;
}