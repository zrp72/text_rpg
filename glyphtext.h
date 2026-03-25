#ifndef glyphtext_header
#define glyphtext_header 1

#include <iostream>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define GLYPHTEXT_ALIGN_CENTER 0
#define GLYPHTEXT_ALIGN_LEFT 1
#define GLYPHTEXT_ALIGN_RIGHT 2
#define GLYPHTEXT_ALIGN_JUSTIFY 3

class glyph_styling;
class glyph;
class glyph_hash;
class glyphtext;
class rasterized_glyph;
class packing_grid;

std::unordered_map<glyph, rasterized_glyph, glyph_hash>
rasterize_glyph_atlas(std::filesystem::path, std::vector<glyph>&, 
    std::vector<SDL_Surface*>, std::string);

void
render_glyphtext(SDL_Renderer*, std::unordered_map<std::string, SDL_Texture*>, 
	glyphtext&, std::unordered_map<glyph, rasterized_glyph>&);

#endif