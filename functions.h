#ifndef functions_header
#define functions_header 1
#include "classes.h"
#include <string>

//game_data load_game();

int create_UI_elements(gui_core&, assets&, game_data&);

int create_story_display(gui_core&, assets&, game_data&);

int create_option_display(gui_core&, assets&, game_data&);

void resize_element(UI_element&, gui_core&, assets&);

int launch_game(game_data&, assets&, gui_core&);

int progress_story(game_data&, assets&, gui_core&, int);

std::pair<bool, story_variables> compare_variables_against_player(game_data&, story_variables&);

#endif