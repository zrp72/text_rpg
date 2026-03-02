#ifndef functions_header
#define functions_header 1
#include "classes.h"
#include <string>

game_data load_game();

std::map<std::string, UI_element> create_UI_elements(gui_core&, game_data);

int launch_game(game_data&, std::map<std::string, UI_element>&, gui_core&);

int run_story(game_data&);

std::string lowercase_string(std::string);

#endif