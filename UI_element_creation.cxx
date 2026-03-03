#include <iostream>
#include <filesystem>

#include "functions.h"

#define debug(x) std::cout << x

UI_element 
create_story_display(gui_core& gui, assets& assets, game_data& data){
    UI_element element;
    const char* story_text = data.story[data.story_location].text.c_str();

    element.add_text(gui, 
        story_text, 
        assets.fonts["roboto_light"], 
        SDL_Color(255, 255, 255, 255), 
        relative_rect(.15, .1, .7, .5), 
        .15, .1, .7);

    element.determine_area();
    element.visible = true;
    return element;
}

UI_element
create_option_display(gui_core& gui, assets& assets, game_data& data){
    UI_element element;

    int column = 0;
    int row = 0;
    float column_w = .3;
    float column_h = .05;

    int iteration = 1;
    for(auto& option : data.story[data.story_location].options){
        if(column > 1){row++; column = 0;}

        float x_pos = .15 + .4 * column;
        float y_pos = .7 + .1 * row;
        relative_rect mask(x_pos, y_pos, column_w, column_h);

        const char* prompt = option.prompt.c_str();
        element.add_text(gui, 
            prompt, 
            assets.fonts["roboto_light"], 
            SDL_Color(255, 255, 255, 255), 
            mask, 
            x_pos, y_pos, .3);

        element.add_texture(gui, 
            assets.textures["option_border"], 
            //using the entire window as source rect, not intended, fix
            relative_rect(), 
            relative_rect(x_pos -.05, y_pos, .4, .05), 
            [=](std::map<std::string, UI_element> elements, game_data& data){
                progress_story(elements, data, iteration);
            });
        ++column;
        ++iteration;
    }

    element.mask = relative_rect(.1, .7, .85, .3);

    element.determine_area();
    element.visible = true;
    return element;
}

std::map<std::string, UI_element>
create_UI_elements(gui_core& gui, assets& assets, game_data& data){
    std::map<std::string, UI_element> elements;

    elements["story_display"] = create_story_display(gui, assets, data);
    elements["option_display"] = create_option_display(gui, assets, data);

    return elements;
}