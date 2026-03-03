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

UI_element
create_option_display(gui_core& gui, game_data data){
    UI_element element;

    int column = 0;
    int row = 0;
    float column_w = .3;
    float column_h = .05;

    for(auto& option : data.story[data.story_location].options){
        if(column > 1){row++; column = 0;}

        float x_pos = .15 + .4 * column;
        float y_pos = .7 + .1 * row;
        relative_rect mask(x_pos, y_pos, column_w, column_h);

        const char* prompt = option.prompt.c_str();
        element.add_text(gui, 
            prompt, 
            "roboto_light", 
            SDL_Color(255, 255, 255, 255), 
            mask, 
            x_pos, y_pos, .3);

        ++column;
    }

    element.mask = relative_rect(.1, .7, .85, .3);

    element.determine_area();
    element.visible = true;
    return element;
}

std::map<std::string, UI_element>
create_UI_elements(gui_core& gui, game_data data){
    std::map<std::string, UI_element> elements;

    elements["story_display"] = create_story_display(gui, data);
    elements["option_display"] = create_option_display(gui, data);

    return elements;
}