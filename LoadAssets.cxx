#include "functions.h"

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
	load_font("roboto_light(bold)", "Roboto-Bold.ttf");
	load_font("roboto_light(italic)", "Roboto-LightItalic.ttf");
	load_font("roboto_light(bold-italic)", "Roboto-BoldItalic.ttf");


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
	SDL_Surface* atlas = SDL_CreateSurface(512, 512, SDL_PIXELFORMAT_ARGB8888);

	SDL_Rect dst(0, 0, 0, 0);

	for(auto& g : passed_glyphs){
		std::string font_tag = g.font;
		if(g.bold && !g.italic){font_tag += "(bold)";}
		if(!g.bold && g.italic){font_tag += "(italic)";}
		if(g.bold && g.italic){font_tag += "(bold-italic)";}
		TTF_Font* font = fonts[font_tag];

		if(!TTF_FontHasGlyph(font, g.unicode)){
			debug(font << "does not contain glyph" << g.unicode << '\n');
		}else{

			rasterized_glyph_info rg;

			TTF_GetGlyphMetrics(font, g.unicode,
				&rg.min_x,
				&rg.max_x,
				&rg.min_y,
				&rg.max_y,
				&rg.advance);

			SDL_Surface* s = TTF_RenderGlyph_Blended(font, g.unicode, g.color);

			if(dst.x + s->w > atlas->w){
				dst.y += TTF_GetFontHeight(font);
				dst.x = 0;
			}

			if(dst.y + s->h > atlas->h){
				textures[atlas_tag] = SDL_CreateTextureFromSurface(gui.renderer, atlas);
				atlas_iteration += 1;
				atlas_tag = "glyph_atlas" + std::to_string(atlas_iteration);
				SDL_ClearSurface(atlas, 0, 0, 0, 255);
				dst.x = 0;
				dst.y = 0;
			}

			SDL_BlitSurface(s, NULL, atlas, &dst);
			glyphs[g] = {atlas_tag, SDL_FRect(dst.x, dst.y, s->w, s->h)};
			dst.x += s->w;
			SDL_DestroySurface(s);
		}
	}

	textures["glyph_atlas" + std::to_string(atlas_iteration)] = SDL_CreateTextureFromSurface(gui.renderer, atlas);
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