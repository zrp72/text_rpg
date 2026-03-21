#include <iostream>
#include <filesystem>
#include <sdl3/SDL_filesystem.h>

#include "functions.h"

#define debug(x)  std::cout << x

int main(int argc, char** argv) {
	try {
		debug("Original directory: " << std::filesystem::current_path() << '\n');
		std::string base_path = SDL_GetBasePath();
		std::filesystem::current_path(base_path);
		debug("New directory: " << std::filesystem::current_path() << '\n');

		files files;

		game_data loaded_data("start");
		loaded_data.parse_story_file(files);

		gui_core gui;

		assets assets(gui);
		assets.rasterize_glyph_atlas(loaded_data.story.used_glyphs, gui);

		if(create_UI_elements(gui, assets, loaded_data)){
			debug("Failed to create UI elements");
			return 1;
		}

		if(launch_game(loaded_data, assets, gui)){
			debug("Failed to launch game");
			return 1;
		}

	}catch (const std::exception &a){
		return 1;
	}
	return 0;
}