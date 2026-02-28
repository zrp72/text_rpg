#include <iostream>
#include <map>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

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

class relative_rect {
    public:
    SDL_Rect rect;
	//relative values are 0-1
	//ex: 0.3
    float relative_x = 1;
    float relative_y = 1;
    float relative_w = 1;
    float relative_h = 1;

    relative_rect(){};
    relative_rect(float x, float y, float w, float h) : relative_x(x), relative_y(y), relative_w(w), relative_h(h){}
};

class relative_text {
	public:
	TTF_Text* text;
	SDL_Rect bounds;
	relative_rect mask;
	float relative_x;
	float relative_y;
	float relative_wrap_width;
	float width;
	float height;
	float x;
	float y;
	int scroll_y;

	relative_text(){};
};

class relative_texture {
	public:
	SDL_Texture* texture;
	relative_rect srcrect;
	relative_rect dstrect;
};

class UI_element {
    public:
    std::vector<relative_texture> texture;
	std::vector<relative_text> rel_text;
    bool visible = false;

    UI_element(){};
};
