#include "classes.h"

bool glyph_styling::operator==(const glyph_styling& other){
	if(font != other.font){return false;}
	if(color.r != other.color.r){return false;}
	if(color.g != other.color.g){return false;}
	if(color.b != other.color.b){return false;}
	if(color.a != other.color.a){return false;}
	if(bold != other.bold){return false;}
	if(italic != other.italic){return false;}
	if(size != other.size){return false;}
	return true;
}

bool glyph::operator==(const glyph& other){
	if(unicode != other.unicode){return false;}
	if(style != other.style){return false;}
	return true;
}

bool glyph::operator<(const glyph& g) const{
	if(unicode != g.unicode){return unicode < g.unicode;}
	return false;
}

size_t glyph_hash::operator()(const glyph& g){
	size_t u = g.unicode;
	size_t f = std::hash<std::string>{}(g.style.font);
	size_t c = g.style.color.r + g.style.color.g + g.style.color.b + g.style.color.a;
	size_t b = std::hash<bool>{}(g.style.bold);
	size_t i = std::hash<bool>{}(g.style.italic);
	return u + f + c + b + i + g.style.size;
}