#include "functions.h"

assets::assets(gui_core& gui){

    std::filesystem::path original_path(std::filesystem::current_path());

    auto load_font = [&](std::string font_name, const char* font_path){
        fonts[font_name] = TTF_OpenFont(font_path, 24);
        if(!fonts[font_name]){
            std::cerr << "Failed to load font: " << SDL_GetError() << '\n';
        }
        TTF_SetFontWrapAlignment(fonts[font_name], TTF_HORIZONTAL_ALIGN_CENTER);
    };

    std::filesystem::current_path("ttf files");

    load_font("roboto_light", "Roboto-Light.ttf");

    std::filesystem::current_path(original_path / "assets");
    textures["option_border"] = IMG_LoadTexture(gui.renderer, "option_border.png");
    if (!textures["option_border"]) {
        std::cerr << "Failed to load texture: " << SDL_GetError() << '\n';
    }
    SDL_SetTextureBlendMode(textures["option_border"], SDL_BLENDMODE_BLEND);

    std::filesystem::current_path(original_path);
}