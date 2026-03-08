#ifndef classes_header
#define classes_header 1
#include <iostream>
#include <map>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <functional>
#include <filesystem>

class item_instance {
	public:
	std::string item_ID;
	int quantity;
};

class character {
	public:
	std::map<std::string, int> attributes = {
		{"Reputation", 0},
		{"Strenth", 0},
		{"Agility", 0}
	};
	std::string lineage;
	std::vector<item_instance> inventory;
	int inventory_size = 30;
	std::map<std::string, std::vector<item_instance>> stashed_items;
	std::map<std::string, bool> decisions;

	character():attributes(){};
};

class option {
	public:
	std::string prompt;
	std::string dest_label;
	std::map<std::string, int> stat_change_map;
	std::map<std::string, int> item_change_map;

	option(std::string p, std::string dest) : prompt(p), dest_label(dest){}
};

class paragraph{
	public:
	std::string text;
	std::map<std::string, int> stat_change_map;
	std::map<std::string, int> item_change_map;
	std::vector<option> options;
	//for nexuses
	bool nexus = false;
	int visits = 0;

	paragraph() : text(""){}
	paragraph(std::string t) : text(t){}
	paragraph(std::string t, bool n) : text(t), nexus(n){}

	void display() {
		std::cout << text << '\n';
		int n = 1;
		for (const option &o : options) {
			std::cout << "  " << n++ << ". " << o.prompt << '\n';
		}
	}
};

class item {
	public:
	std::string ID;
	std::string item_type;
	std::map<std::string, int> stat_bonus;

	item(): ID(""){}
	item(std::string i) : ID(i){}
};

class files {
	public:
	std::string story;
	std::string items;

	files(std::string s, std::string i): story(s), items(i){};
};

class game_data {
	public:
	std::map<std::string, paragraph> story;
	std::string story_location;
	std::map<std::string, item> items;
	character player;

	game_data(auto s, std::string l, auto i, character p)
	: story(s), story_location(l), items(i), player(p){};
};

class gui_core {
	public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	TTF_TextEngine* text_engine;

	gui_core(){
		if(!SDL_Init(SDL_INIT_VIDEO)){
			throw std::runtime_error(SDL_GetError());
		}

		if(!TTF_Init()){
			throw std::runtime_error(SDL_GetError());
		}

		window = SDL_CreateWindow("TextRPG", 800, 600, SDL_WINDOW_RESIZABLE);
		if(!window){
			throw std::runtime_error(SDL_GetError());
		}

		renderer = SDL_CreateRenderer(window, NULL);
		if(!renderer){
			throw std::runtime_error(SDL_GetError());
		}

		text_engine = TTF_CreateRendererTextEngine(renderer);
		if(!text_engine){
			throw std::runtime_error(SDL_GetError());
		}
	}

	~gui_core(){
		TTF_DestroySurfaceTextEngine(text_engine);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
	}
};

class assets;

class relative_rect {
	public:
	SDL_Rect SDL_rect;
	//x position relative to screen size, from left to right, 0... 1.
	float relative_x = 0;
	//y position relative to screen size, from top to bottom, 0... 1.
	float relative_y = 0;
	//width relative to screen size, 0... 1.
	float relative_w = 1;
	//height relative to screen size, 0... 1.
	float relative_h = 1;

	relative_rect(){}
	relative_rect(float x, float y, float w, float h) : relative_x(x), relative_y(y), relative_w(w), relative_h(h){}
};

class relative_text {
	public:
	TTF_Text* TTF_text = nullptr;
	std::string text;
	std::string font_ID;
	SDL_Color color;
	relative_rect mask;
	//x position relative to screen size, from left to right, 0... 1.
	float relative_x;
	//y position relative to screen size, from top to bottom, 0... 1.
	float relative_y;
	//wrap width relative to screen size, 0... 1.
	float relative_wrap_width;
	float width;
	float height;
	float x;
	float y;
	float wrap_width;
	int scroll_y = 0;

	relative_text(){};
	relative_text(std::string t, std::string f, SDL_Color c, relative_rect m, float x, float y, float w)
	: text(t), font_ID(f), color(c), mask(m), relative_x(x), relative_y(y), relative_wrap_width(w){};
};

class relative_texture {
	public:
	std::string texture_ID;
	SDL_Rect srcrect;
	relative_rect dstrect;
	std::function<void(game_data&, assets&, gui_core&)> click_events = nullptr;

	relative_texture(){};
	relative_texture(std::string t, SDL_Rect s, relative_rect d, std::function<void(game_data&, assets&, gui_core&)> c)
	: texture_ID(t), srcrect(s), dstrect(d), click_events(c){};
};

class UI_element {
	public:
	std::vector<relative_texture> texture;
	std::vector<relative_text> text;
	relative_rect mask;
	relative_rect area;
	int scroll_y = 0;
	bool visible = false;

	UI_element(){}

	void determine_area();
};

class assets {
	public:
	std::map<std::string, TTF_Font*> fonts;
	std::map<std::string, SDL_Texture*> textures;
	std::map<std::string, UI_element> elements;
	std::vector<TTF_Text*> created_texts;

	assets(gui_core&);

	~assets(){
		for(auto& t : created_texts){
			TTF_DestroyText(t);
		}
		for(auto& [tag, f] : fonts){
			TTF_CloseFont(f);
		}
		for(auto& [tag, t] : textures){
			SDL_DestroyTexture(t);
		}
	}

	void add_element(std::string tag, UI_element& e, gui_core& gui){
		e.determine_area();
		for(auto& t : e.text){
			TTF_Text* text = TTF_CreateText(gui.text_engine, fonts[t.font_ID], t.text.data(), 0);
			TTF_SetTextColor(text, t.color.r, t.color.g, t.color.b, t.color.a);
			created_texts.push_back(text);
			t.TTF_text = text;
		}
		elements.insert_or_assign(tag, e);
	}

	SDL_Rect get_texture_srcrect(std::string tag){
		float width, height;
		SDL_GetTextureSize(textures[tag], &width, &height);
		SDL_Rect srcrect(0, 0, width, height);
		return srcrect;
	}
};

#endif