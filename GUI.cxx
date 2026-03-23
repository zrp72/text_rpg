/*
Functions to likely use in the future:
TTF_GetStringSize

*/

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>

#include "functions.h"

#define debug(x)  std::cout << x

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
		resize_rect(text.canvas, gui.window);

		int max_scroll = text.canvas.SDL_rect.h - text.mask.SDL_rect.h;
		max_scroll = std::max(0, max_scroll);
		text.scroll_y = std::clamp(text.scroll_y, 0, max_scroll);
	}

	int max_scroll = obj.area.SDL_rect.h - obj.mask.SDL_rect.h;
	max_scroll = std::max(0, max_scroll);
	obj.scroll_y = std::clamp(obj.scroll_y, 0, max_scroll);
}

void
render_glyph_text(SDL_Renderer* renderer, assets& assets, relative_text text, int scroll){
	//TODO: Add kerning
	std::vector<std::vector<glyph>> words;
	std::vector<glyph> word;
	int ascent = 0;
	int descent = 0;
	int default_space = 0;
	for(glyph& g : text.text){

		auto& glyph_data = assets.glyphs.at(g);
		ascent = std::max(ascent, glyph_data.ascent);
		descent = std::min(descent, glyph_data.descent);

		if(g.unicode == '\n'){
			word.push_back(g);
			words.push_back(word);
			word.clear();
		}else if(g.unicode == ' '){
			default_space = std::max(default_space, glyph_data.advance);
			if(!word.empty()){
				words.push_back(word);
				word.clear();
			}
		}else{
			word.push_back(g);
		}
	}
	if(!word.empty()){
		words.push_back(word);
	}

	int font_height = ascent - descent;
	int line_spacing = font_height * 0.3;
	int max_line_size = text.canvas.SDL_rect.w * 0.9;
	int line_size = 0;
	int word_count = 0;
	int line = 1;
	int pen_x;
	int pen_y;

	auto render_glyph = [&](glyph& glyph){
		auto& glyph_data = assets.glyphs.at(glyph);
		SDL_Texture* atlas = assets.textures.at(glyph_data.atlas_tag);
		SDL_FRect& srcrect = glyph_data.position_on_atlas;
		SDL_FRect dstrect(pen_x + glyph_data.min_x, pen_y - glyph_data.ascent,
			srcrect.w, srcrect.h);
		SDL_RenderTexture(renderer, atlas, &srcrect, &dstrect);
		pen_x += glyph_data.advance;
	};

	for(int w = 0; w < words.size(); ++w){
		int word_size = 0;
		for(auto& glyph : words.at(w)){
			word_size += assets.glyphs.at(glyph).advance;
		}

		//TODO: Force line break if glyph equals newline
		if(line_size + word_size <= max_line_size){

			line_size += word_size;
			word_count += 1;

		}else{

			int spacing = 0;
			if(word_count > 1){
				spacing = (text.canvas.SDL_rect.w - line_size) / (word_count - 1);
			}

			pen_x = text.canvas.SDL_rect.x;
			pen_y = text.canvas.SDL_rect.y + line * (ascent + line_spacing) - scroll;

			if(word_count == 0){
				//TODO: Write some sort of word splitter
			}else{
				while(word_count != 0){
					for(auto& glyph : words.at(w - word_count)){
						if(!assets.glyphs.contains(glyph)){
							debug("Glyph \"" << char(glyph.unicode) << "\" has not been rasterized.\n");
						}else{
							render_glyph(glyph);
						}
					}
					word_count -= 1;
					pen_x += spacing;
				}
				line += 1;
				line_size = word_size;
				word_count = 1;
			}
		}
	}

	pen_x = text.canvas.SDL_rect.x;
	pen_y = text.canvas.SDL_rect.y + line * (ascent + line_spacing) - scroll;
	while(word_count > 0){
		for(auto& glyph : words.at(words.size() - word_count)){
			if(!assets.glyphs.contains(glyph)){
				debug("Glyph \"" << char(glyph.unicode) << "\" has not been rasterized.\n");
			}else{
				render_glyph(glyph);
			}
		}
		word_count -= 1;
		pen_x += default_space;
	}
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
	SDL_SetRenderTarget(gui.renderer, NULL);

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

			render_glyph_text(gui.renderer, assets, t, t.scroll_y + obj.scroll_y);
		}
	}
	SDL_SetRenderClipRect(gui.renderer, NULL);
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
							int max_scroll = text.canvas.SDL_rect.h - text.mask.SDL_rect.h;
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
		SDL_SetRenderTarget(gui.renderer, NULL);
		SDL_RenderClear(gui.renderer);

		for (const auto &[tag, obj] : assets.elements){
			if(obj.visible){
				render_element(obj, gui, assets);
			}
		}

		/*
		SDL_FRect dstrect(0, 0, 50, 50);
		for(auto& glyph : assets.glyphs){
			debug("Rendering glyph " << char(glyph.first.unicode) << '\n');
			auto srcrect = glyph.second.position_on_atlas;
			auto atlas = glyph.second.atlas_tag;
			SDL_RenderTexture(gui.renderer, assets.textures.at(atlas), &srcrect, &dstrect);
			dstrect.y += 50;
			if(dstrect.y > 500){
				dstrect.x += 50;
				dstrect.y = 0;
			}
		}
		*/
		//SDL_RenderTexture(gui.renderer, assets.textures.at("glyph_atlas1"), NULL, NULL);

		SDL_RenderPresent(gui.renderer);
	}

	return 0;
}