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

    if(present_game(loaded_data)){
        debug("Failed to present game");
        return 1;
    }

    return 0;
}