#include "glyphtext.h"
#define debug(x) std::cout << x

class glyph_styling {
	public:
	//FONT SHOULD BE THE FILENAME(can exclude the .ttf)
	std::string font;
	SDL_Color color = {255, 255, 255, 255};
	bool bold = false;
	bool italic = false;
	int size = 24;

	glyph_styling(){}
};

class glyph {
	public:
	uint32_t unicode;
	glyph_styling style;

	glyph(){}
	glyph(uint32_t u, glyph_styling s)
	: unicode(u), style(s){}

	bool operator==(const glyph& other){
		if(unicode != other.unicode){return false;}
		if(style.font != other.style.font){return false;}
		if(style.color.r != other.style.color.r){return false;}
		if(style.color.g != other.style.color.g){return false;}
		if(style.color.b != other.style.color.b){return false;}
		if(style.color.a != other.style.color.a){return false;}
		if(style.bold != other.style.bold){return false;}
		if(style.italic != other.style.italic){return false;}
		if(style.size != other.style.size){return false;}
		return true;
	}

	bool operator<(const glyph& other){
		if(unicode < other.unicode){return true;}
		if(style.color.r < other.style.color.r){return true;}
		if(style.color.g < other.style.color.g){return true;}
		if(style.color.b < other.style.color.b){return true;}
		if(style.color.a < other.style.color.a){return true;}
		if(style.bold != other.style.bold){return true;}
		if(style.italic != other.style.italic){return true;}
		if(style.size < other.style.size){return true;}
		return false;
	}
};

class glyph_hash {
	public:
	size_t operator()(const glyph& glyph){
		//TODO: Learn how to write a good hash function
		size_t u = glyph.unicode;
		size_t r = glyph.style.color.r << 1;
		size_t g = glyph.style.color.g * 5;
		size_t b = glyph.style.color.b / 30;
		size_t a = glyph.style.color.a + 44;
		return u + r + g + b + a;
	}
};

class glyphtext {
	public:
	//Word and line are mainly used for refreshing layout
	class word {
		public:
		std::vector<glyph> glyphs;
		int length = 0;

		word(){}
		void clear(){
			glyphs.clear();
			length = 0;
		}
	};
	class line {
		public:
		std::vector<glyph> glyphs;
		//LENGTH IS ONLY THE LENGTH OF COMBINED GLYPHS,
		//IT DOES NOT INCLUDE THE LENGTH OF WHITESPACE.
		int length = 0;
		int spaces = -1;
		
		line(){}
		void clear(){
			glyphs.clear();
			length = 0;
			spaces = -1;
		}
	};

	std::vector<glyph> glyphs;
	std::vector<SDL_FRect> layout;
	int height;

	glyphtext(){}
	void refresh_layout(std::unordered_map<glyph, rasterized_glyph>&, int, int);
};

class rasterized_glyph {
	public:
	std::string atlas_tag;
	SDL_FRect position_on_atlas;
	uint32_t unicode;
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int advance;
	int descent;
	int ascent;

	rasterized_glyph(){}
};

class packing_grid {
	public:
	size_t width = 0;
	size_t height = 0;
	std::vector<std::vector<bool>> pixels;

	//Takes width and height to create a vector<vector<bool>>(a grid of pixels either available or used)
	packing_grid(size_t w, size_t h):width(w), height(h){
		for(int x = 0; x < width; ++x){
			std::vector<bool> column;
			for(int y = 0; y < height; ++y){
				column.push_back(false);
			}
			pixels.push_back(column);
		}
	}

	//Returns an xy pixel location for where the object can fit from its top left corner.
	//Padding should always be an even number.
	//If no position is found, the returned point will be -1, -1
	std::pair<int, int>
	find_space(int w, int h, int padding){
		for(int x = 0; x < width; ++x){
			for(int y = 0; y < height; ++y){
				if(pixels.at(x).at(y)){
					continue;
				}else{
					bool viable = true;
					for(int px = 0; px < w + padding && x + px < width; ++px){
						if(x + px >= width){
							viable = false;
							break;
						}
						for(int py = 0; py < h + padding; ++py){
							if(pixels.at(x + px).at(y + py)){
								viable = false;
								break;
							}
							if(y + py >= height){
								viable = false;
								break;
							}
						}
						if(!viable){
							break;
						}
					}
					if(viable){
						return {x + (padding / 2), y + (padding / 2)};
					}
				}
			}
		}
		return {-1, -1};
	}

	//Takes the x, y, width, and height of the space being used.
	//Will return false if given area goes beyond pixel grid bounds.
	//Will print a debug if it attempts to use a pixel already used.
	bool use_pixels(int x, int y, int w, int h){
		for(int px = 0; px < w; ++px){
			if(x + px >= width){return false;}
			for(int py = 0; py < h; ++py){
				if(y + py >= height){return false;}
				pixels.at(x + px).at(y + py) = true;
			}
		}
		return true;
	}
};

//The passed directory is where all ttf font files should reside.
//This function assumes that glyph font names will match their filenames.
//
//Takes a vector of every glyph to rasterize and a
//vector of SDL_Surface(s) to add the atlas(s) to.
//
//Atlas naming defines what the rasterized glyphs will store their atlas tags as.
//Given atlas name will be followed by the number related to its iteration, starting at 1.
//
//Return an unordered map of glyphs to their rasterized positions and data.
std::unordered_map<glyph, rasterized_glyph, glyph_hash>
rasterize_glyph_atlas(std::filesystem::path directory, std::vector<glyph>& glyphs, std::vector<SDL_Surface*> container, std::string atlas_naming){

	std::filesystem::path original_directory = SDL_GetBasePath();
	std::filesystem::current_path(directory);
	std::unordered_map<glyph, rasterized_glyph, glyph_hash> rasterized_glyphs;
	std::unordered_map<std::string, TTF_Font*> cached_fonts;

	int atlas_iteration = 1;
	std::string atlas_tag = atlas_naming + std::to_string(atlas_iteration);

	//NOTE: If atlas size is ever changed, 1024 is also used later in this function.
	//TODO: Add atlas width and size to function params.
	packing_grid packing_container(1024, 1024);
	SDL_Surface* atlas = SDL_CreateSurface(1024, 1024, SDL_PIXELFORMAT_RGBA8888);
	for(auto& glyph : glyphs){

		if(!cached_fonts.contains(glyph.style.font)){
			std::filesystem::path path(glyph.style.font);
			if(!path.has_extension()){
				path += ".ttf";
			}else if(path.extension() != ".ttf"){
				debug("Attempted to rasterize glyph with unknown font file type(not .ttf):" << path << '\n');
				return rasterized_glyphs;
			}
			TTF_Font* font = TTF_OpenFont(path.string().data(), 24);
			cached_fonts.emplace(glyph.style.font, font);
		}

		TTF_Font* font = cached_fonts.at(glyph.style.font);
		if(glyph.style.bold){TTF_SetFontStyle(font, TTF_STYLE_BOLD);}
		if(glyph.style.italic){TTF_SetFontStyle(font, TTF_STYLE_ITALIC);}
		if(glyph.style.bold && glyph.style.italic){TTF_SetFontStyle(font, TTF_STYLE_BOLD | TTF_STYLE_ITALIC);}
		TTF_SetFontSize(font, glyph.style.size);
		SDL_Surface* g = TTF_RenderGlyph_Blended(font, glyph.unicode, glyph.style.color);
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

		rasterized_glyph rg;
		TTF_GetGlyphMetrics(font, glyph.unicode,
			&rg.min_x,
			&rg.max_x,
			&rg.min_y,
			&rg.max_y,
			&rg.advance
		);
		rg.descent = TTF_GetFontDescent(font);
		rg.ascent = TTF_GetFontAscent(font);

		auto dst = packing_container.find_space(g->w, g->h, 2);
		if(dst.first == -1){

			container.push_back(atlas);
			atlas = SDL_CreateSurface(1024, 1024, SDL_PIXELFORMAT_RGBA8888);
			atlas_iteration += 1;
			atlas_tag = atlas_naming + std::to_string(atlas_iteration);

			packing_container = packing_grid(1024, 1024);
			dst = packing_container.find_space(g->w, g->h, 2);
			if(dst.first == -1){
				debug("Glyph is larger than glyph atlas bounds.");
				return rasterized_glyphs;
			}
		}
		if(!packing_container.use_pixels(dst.first, dst.second, g->w, g->h)){
			debug("Attempted to pack glyph outside pixel bounds.");
		}
		SDL_Rect dstrect(dst.first, dst.second, 0, 0);
		SDL_BlitSurface(g, NULL, atlas, &dstrect);

		rg.position_on_atlas = SDL_FRect(dst.first, dst.second, g->w, g->h);
		rg.atlas_tag = atlas_tag;
		rg.unicode = glyph.unicode;

		rasterized_glyphs.emplace(glyph, rg);
	}

	for(std::pair<std::string, TTF_Font*> font : cached_fonts){
		TTF_CloseFont(font.second);
	}
	std::filesystem::current_path(original_directory);
	return rasterized_glyphs;
}


//Lays out glyphs with SDL_FRects's starting at their top-left corners.
//First point may not always be 0, 0
//Points are NOT given for whitespace(or atleast shouldnt be).
//
//Alignments are:
//GLYPHTEXT_ALIGN_CENTER
//GLYPHTEXT_ALIGN_LEFT
//GLYPHTEXT_ALIGN_RIGHT
//GLYPHTEXT_ALIGN_JUSTIFY
void
glyphtext::refresh_layout(std::unordered_map<glyph, rasterized_glyph>& ras_info, int w, int alignment){
	layout.clear();
	int ascent = 0;
	int descent = 0;
	int default_space = 0;

	//TODO: Make sure every type of whitespace is handled
	std::vector<glyphtext::word> words;
	glyphtext::word word;
	for(auto& g : this->glyphs){
		ascent = std::max(ascent, ras_info.at(g).ascent);
		descent = std::min(descent, ras_info.at(g).descent);

		switch(ras_info.at(g).unicode){
		case ' ':
			default_space = std::max(default_space, ras_info.at(g).advance);
			if(!word.glyphs.empty()){
				words.push_back(word);
				word.clear();
			}
			continue;

		case '\n':
			word.glyphs.push_back(g);
			words.push_back(word);
			word.clear();
			continue;

		default:
			word.glyphs.push_back(g);
			word.length += ras_info.at(g).advance;
		}
	}
	if(!word.glyphs.empty()){
		words.push_back(word);
		word.clear();
	}

	std::vector<glyphtext::line> lines;
	glyphtext::line line;
	//Moves the current line to lines before clearing current line.
	auto next_line = [&](){
		lines.push_back(line);
		line.clear();
	};

	for(auto& word : words){
		if(line.length + word.length > w){
			next_line();

		//Ensuring enough room for spaces.
		}else if(alignment == GLYPHTEXT_ALIGN_JUSTIFY){
			//TODO: Handle undefined behavior when spaces is equal to -1 & 0
			if((line.length + word.length) / line.spaces < default_space){
				next_line();
			}
		}else if(line.length + (default_space * (line.spaces + 1)) + word.length > w){
			next_line();
		}

		//Handling words too long for a single line.
		if(line.length + word.length > w){
			for(auto& glyph : word.glyphs){
				if(line.length + ras_info.at(glyph).advance > w){
					next_line();
				}
				line.glyphs.push_back(glyph);
				line.length += ras_info.at(glyph).advance;
			}
			line.spaces += 1;
			continue;
		}

		if(word.glyphs.back().unicode == '\n'){
			word.glyphs.pop_back();
			line.glyphs.append_range(word.glyphs);
			next_line();
		}else{
			line.glyphs.append_range(word.glyphs);
			line.length += word.length;
		}
	}
	if(!line.glyphs.empty()){
		lines.push_back(line);
		line.clear();
	}
	
	int font_height = ascent - descent;
	//font_height will never change, but pen_y moves per line.
	int pen_y = font_height + descent;
	int pen_x = 0;
	int line_padding = 1.4;

	auto append_layout = [&](rasterized_glyph& rg){
		layout.push_back(
			SDL_FRect(pen_x + rg.min_x, 
			pen_y - rg.ascent, 
			rg.position_on_atlas.w, 
			rg.position_on_atlas.h)
		);
		pen_x += rg.advance;
	};

	for(auto& line : lines){
		if(alignment == GLYPHTEXT_ALIGN_CENTER){
			pen_x = (w - (line.length + (line.spaces * default_space))) / 2;
			for(auto& glyph : line.glyphs){
				if(glyph.unicode == ' '){
					pen_x += default_space;
				}else{
					append_layout(ras_info.at(glyph));
				}
			}
		}else if(alignment == GLYPHTEXT_ALIGN_LEFT){
			for(auto& glyph : line.glyphs){
				if(glyph.unicode == ' '){
					pen_x += default_space;
				}else{
					append_layout(ras_info.at(glyph));
				}
			}
		}else if(alignment == GLYPHTEXT_ALIGN_RIGHT){
			pen_x = w - (line.length + (line.spaces * default_space));
			for(auto& glyph : line.glyphs){
				if(glyph.unicode == ' '){
					pen_x += default_space;
				}else{
					append_layout(ras_info.at(glyph));
				}
			}
		}else if(alignment == GLYPHTEXT_ALIGN_JUSTIFY){
			if(line.spaces == 0){
				for(auto& glyph : line.glyphs){
					append_layout(ras_info.at(glyph));
				}
			}else{
				int spacing = (w - line.length) / line.spaces;
				for(auto& glyph : line.glyphs){
					if(glyph.unicode == ' '){
						pen_x += spacing;
					}else{
						append_layout(ras_info.at(glyph));
					}
				}
			}
		}
		pen_y += font_height * line_padding;
		pen_x = 0;
	}
}

void
render_glyphtext(SDL_Renderer* renderer, std::unordered_map<std::string, SDL_Texture*> atlas_map, 
	glyphtext& text, std::unordered_map<glyph, rasterized_glyph>& ras_info){
	int l = 0;
	for(auto& glyph : text.glyphs){
		if(std::isalpha(glyph.unicode)){
			if(l < text.layout.size()){
				SDL_RenderTexture(renderer, atlas_map.at(ras_info.at(glyph).atlas_tag), 
				&ras_info.at(glyph).position_on_atlas, &text.layout.at(l));
				l += 1;
			}else{
				debug("Layout does not contain position for glyph.\n");
			}
		}
	}
}