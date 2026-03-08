#include "functions.h"

void
UI_element::determine_area(){
    float min_x = 1;
    float min_y = 1;
    float top_w = 0;
    float top_h = 0;

    auto resize = [&](float x, float y, float w, float h){
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        top_w = std::max(top_w, x + w);
        top_h = std::max(top_h, y + h);
    };

    for(auto &tex : texture){
        auto t = tex.dstrect;
        resize(t.relative_x, t.relative_y, t.relative_w, t.relative_h);
    }
    for(auto &txt : text){
        resize(txt.relative_x, txt.relative_y, txt.relative_wrap_width, txt.mask.relative_h);
    }

    area = {min_x, min_y, top_w - min_x, top_h - min_y};
}