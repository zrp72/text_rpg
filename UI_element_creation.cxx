#include <iostream>
#include <filesystem>

#include "functions.h"

#define debug(x) std::cout << x

int
create_story_display(gui_core& gui, assets& assets, game_data& data){
    UI_element element;
    std::string story_text = data.story[data.story_location].text;

    element.text.push_back(relative_text(
        story_text,
        "roboto_light",
        SDL_Color(255, 255, 255, 255),
        relative_rect(.15, .1, .7, .5),
        .15, .1, .7
    ));

    element.visible = true;
    assets.add_element("story_display", element);
    return 0;
}

int
create_option_display(gui_core& gui, assets& main_assets, game_data& data){
    UI_element element;

    int column = 0;
    int row = 0;
    float column_w = .3;
    float column_h = .05;

    int iteration = 1;
    for(auto& option : data.story[data.story_location].options){
        debug(option.prompt << '\n');
        debug("Option iteration: " << iteration << '\n');
        if(column > 1){++row; column = 0;}

        float x_pos = .15 + .4 * column;
        float y_pos = .7 + .1 * row;
        relative_rect mask(x_pos, y_pos, column_w, column_h);

        std::string prompt = option.prompt;
        element.text.push_back(relative_text(
            prompt, 
            "roboto_light", 
            SDL_Color(255, 255, 255, 255), 
            mask, 
            x_pos, y_pos, .3
        ));

        element.texture.push_back(relative_texture(
            "option_border",
            main_assets.get_texture_srcrect("option_border"),
            relative_rect(x_pos -.05, y_pos, .4, .05),
            [=](game_data& data, assets& assets, gui_core& gui){
                progress_story(data, assets, gui, iteration);
            }
        ));

        ++column;
        ++iteration;
    }

    element.mask = relative_rect(.1, .7, .85, .3);
    element.visible = true;

    //doesn't exist on second run???
    debug("Element text 0" << element.text[0].text << '\n');

    main_assets.add_element("option_display", element);

    return 0;
}

int
create_UI_elements(gui_core& gui, assets& assets, game_data& data){

    create_story_display(gui, assets, data);
    create_option_display(gui, assets, data);

    return 0;
}