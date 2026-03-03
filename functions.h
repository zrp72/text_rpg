#ifndef functions_header
#define functions_header 1
#include "classes.h"
#include <string>

game_data load_game();

std::map<std::string, UI_element> create_UI_elements(gui_core&, assets&, game_data&);

int launch_game(game_data&, assets&, gui_core&);

int progress_story(std::map<std::string, UI_element>&, game_data&, int);

std::string lowercase_string(std::string);

#endif