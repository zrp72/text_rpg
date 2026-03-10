#include <iostream>
#include <filesystem>

#include "functions.h"

#define debug(x) std::cout << x

void
UI_element::determine_area(){
	float min_x = 1;
	float min_y = 1;
	float top_w = 0;
	float top_h = 0;

	auto resize = [&](float x, float y, float w, float h){
		min_x = std::min(min_x, x);
		min_y = std::min(min_y, y);
		top_w = std::max(top_w, x + w);
		top_h = std::max(top_h, y + h);
	};

	for(auto &tex : texture){
		auto t = tex.dstrect;
		resize(t.relative_x, t.relative_y, t.relative_w, t.relative_h);
	}
	for(auto &txt : text){
		resize(txt.relative_x, txt.relative_y, txt.relative_wrap_width, txt.mask.relative_h);
	}

	area = {min_x, min_y, top_w - min_x, top_h - min_y};
}

int
create_story_display(gui_core& gui, assets& assets, game_data& data){
	UI_element element;
	std::string story_text = data.story[data.story_location].text;

	element.text.push_back(relative_text(
		story_text,
		"roboto_light",
		SDL_Color(255, 255, 255, 255),
		relative_rect(.15, .1, .7, .5),
		.15, .1, .7
	));

	element.visible = true;
	assets.add_element("story_display", element, gui);
	resize_element(assets.elements["story_display"], gui.window, assets);
	return 0;
}

int
create_option_display(gui_core& gui, assets& main_assets, game_data& data){
	UI_element element;

	int column = 0;
	int row = 0;
	float column_w = .3;
	float column_h = .05;

	int iteration = 1;
	for(auto& option : data.story[data.story_location].options){
		debug("Option iteration: " << iteration << '\n');
		debug(option.prompt << '\n');
		if(column > 1){++row; column = 0;}

		float x_pos = .15 + .4 * column;
		float y_pos = .7 + .1 * row;
		relative_rect mask(x_pos, y_pos, column_w, column_h);

		std::string prompt = option.prompt;
		element.text.push_back(relative_text(
			prompt,
			"roboto_light",
			SDL_Color(255, 255, 255, 255),
			mask,
			x_pos, y_pos, .3
		));

		element.texture.push_back(relative_texture(
			"option_border",
			main_assets.get_texture_srcrect("option_border"),
			relative_rect(x_pos -.05, y_pos, .4, .05),
			[=](game_data& data, assets& assets, gui_core& gui){
				progress_story(data, assets, gui, iteration);
			}
		));

		++column;
		++iteration;
	}

	element.mask = relative_rect(.1, .7, .85, .3);
	element.visible = true;

	//doesn't exist on second run???
	//debug("Element text 0" << element.text[0].text << '\n');
	debug("Element text vector size: " << element.text.size() << '\n');

	main_assets.add_element("option_display", element, gui);
	resize_element(main_assets.elements["option_display"], gui.window, main_assets);

	return 0;
}

int
create_UI_elements(gui_core& gui, assets& assets, game_data& data){

	create_story_display(gui, assets, data);
	create_option_display(gui, assets, data);

	return 0;
}