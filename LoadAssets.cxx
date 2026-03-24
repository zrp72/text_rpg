#include <filesystem>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include "classes.h"

#define debug(x)  std::cout << x

assets::assets(gui_core& gui){

	std::filesystem::path original_path(std::filesystem::current_path());
	std::filesystem::current_path("ttf files");


	auto load_font = [&](std::string font_name, const char* font_path){
		TTF_Font* font = TTF_OpenFont(font_path, 24);
		if(!font){
			std::cerr << "Failed to load font: " << SDL_GetError() << '\n';
		}
		TTF_SetFontWrapAlignment(font, TTF_HORIZONTAL_ALIGN_CENTER);
		this->fonts[font_name] = font;
	};

	load_font("roboto_light", "Roboto-Light.ttf");


	std::filesystem::current_path(original_path / "assets");

	auto load_texture = [&](std::string texture_name, const char* path){
		SDL_Texture* texture = IMG_LoadTexture(gui.renderer, path);
		if(!texture){
			std::cerr << "Failed to load texture: " << SDL_GetError() << '\n';
		}
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		this->textures[texture_name] = texture;
	};

	load_texture("option_border", "option_border.png");

	std::filesystem::current_path(original_path);
}

void
assets::rasterize_glyph_atlas(std::vector<glyph>& passed_glyphs, gui_core& gui){

	int atlas_iteration = 1;
	std::string atlas_tag = "glyph_atlas" + std::to_string(atlas_iteration);
	SDL_Surface* atlas = SDL_CreateSurface(1024, 1024, SDL_PIXELFORMAT_RGBA8888);
	SDL_SetSurfaceBlendMode(atlas, SDL_BLENDMODE_BLEND);

	int highest_font_height = 0;
	SDL_Rect dst(0, 0, 0, 0);

	for(auto& g : passed_glyphs){
		std::string font_tag = g.style.font;
		TTF_Font* font = fonts.at(font_tag);
		if(g.style.bold && !g.style.italic){TTF_SetFontStyle(font, TTF_STYLE_BOLD);}
		if(!g.style.bold && g.style.italic){TTF_SetFontStyle(font, TTF_STYLE_ITALIC);}
		if(g.style.bold && g.style.italic){TTF_SetFontStyle(font, TTF_STYLE_BOLD | TTF_STYLE_ITALIC);}

		TTF_SetFontSize(font, g.style.size);

		highest_font_height = std::max(highest_font_height, TTF_GetFontHeight(font));

		if(!TTF_FontHasGlyph(font, g.unicode)){
			debug(font << "does not contain glyph" << g.unicode << '\n');
		}else{
			debug("Rasterized glyph: " << char(g.unicode) << '\n');

			rasterized_glyph_info rg;

			TTF_GetGlyphMetrics(font, g.unicode,
				&rg.min_x,
				&rg.max_x,
				&rg.min_y,
				&rg.max_y,
				&rg.advance);

			rg.ascent = TTF_GetFontAscent(font);
			rg.descent = TTF_GetFontDescent(font);

			SDL_Surface* s = TTF_RenderGlyph_Blended(font, g.unicode, g.style.color);
			SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);

			if(dst.x + s->w > atlas->w){
				dst.y += highest_font_height;
				dst.x = 0;
			}

			if(dst.y + s->h > atlas->h){
				textures[atlas_tag] = SDL_CreateTextureFromSurface(gui.renderer, atlas);
				atlas_iteration += 1;
				atlas_tag = "glyph_atlas" + std::to_string(atlas_iteration);
				SDL_ClearSurface(atlas, 0, 0, 0, 0);
				dst.x = 0;
				dst.y = 0;
			}

			SDL_BlitSurface(s, NULL, atlas, &dst);
			SDL_FRect position_on_atlas(dst.x, dst.y, s->w, s->h);
			rg.position_on_atlas = position_on_atlas;
			debug(char(g.unicode) << " is on " << atlas_tag << " at position " << int(position_on_atlas.x) << int(position_on_atlas.y) << int(position_on_atlas.w) << int(position_on_atlas.h) << '\n');
			
			rg.atlas_tag = atlas_tag;
			this->glyphs[g] = rg;
			dst.x += s->w;
			SDL_DestroySurface(s);
		}
	}

	textures[atlas_tag] = SDL_CreateTextureFromSurface(gui.renderer, atlas);
	SDL_DestroySurface(atlas);
}

void
assets::add_element(std::string tag, UI_element& e, gui_core& gui){
	e.determine_area();
	elements.insert_or_assign(tag, e);
}

SDL_Rect
assets::get_texture_srcrect(std::string tag){
	float width, height;
	SDL_GetTextureSize(textures[tag], &width, &height);
	SDL_Rect srcrect(0, 0, width, height);
	return srcrect;
}