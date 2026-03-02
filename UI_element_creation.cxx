#include <iostream>

#include "functions.h"

#define debug(x) std::cout << x

UI_element 
create_story_display(gui_core& gui, game_data data){
    UI_element element;
    const char* story_text = data.story[data.story_location].text.c_str();

    element.add_text(gui, 
        story_text, 
        "roboto_light", 
        SDL_Color(255, 255, 255, 255), 
        relative_rect(.15, .1, .7, .5), 
        .15, .1, .7);

    element.determine_area();
    element.visible = true;
    return element;
}

std::map<std::string, UI_element>
create_UI_elements(gui_core& gui, game_data data){
    std::map<std::string, UI_element> elements;

    elements["story_display"] = create_story_display(gui, data);

    return elements;
}