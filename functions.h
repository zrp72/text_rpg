#ifndef functions_header
#define functions_header 1
#include "classes.h"
#include <string>

game_data load_game();

int present_game(game_data &data);

int run_story(game_data &data);

std::string lowercase_string(std::string);

#endif