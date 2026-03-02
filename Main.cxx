#include <iostream>
#include <sstream>
#include <filesystem>
#include <sdl3/SDL_filesystem.h>

#include "functions.h"

#define debug(x)  std::cout << x 


int main(int argc, char** argv) {
    debug("Original directory: " << std::filesystem::current_path() << '\n');
    std::string base_path = SDL_GetBasePath();
    std::filesystem::current_path(base_path);
    debug("Original directory: " << std::filesystem::current_path() << '\n');

    game_data loaded_data = load_game();
    if(loaded_data.story.empty()){
        debug("Failed to load story");
        return 1;
    }

    try {

        gui_core gui;
        gui.add_font("roboto_light", "ttf files/Roboto-Light.ttf", 24, TTF_HORIZONTAL_ALIGN_CENTER);

        std::map<std::string, UI_element> elements = create_UI_elements(gui, loaded_data);

        if(elements.empty()){
            debug("Failed to create UI elements");
            return 1;
        }

        if(launch_game(loaded_data, elements, gui)){
            debug("Failed to present game");
            return 1;
        }

    }catch (const std::exception &a){
        return 1;
    }
    return 0;
}