#include "core/renderer/sdl/sdl_struct.h"


SDLFont::~SDLFont() {
    if (_font) {
        TTF_CloseFont(_font);
    }
}

glm::vec2 SDLFont::get_size(const std::string& text) {
    int w = 0, h = 0;

    if (_font && TTF_GetStringSize(_font, text.c_str(), text.size(), &w, &h) == 0) {
        return {float(w), float(h)};
    }

    return {0, 0};
}

TTF_Font* SDLFont::get_font() const {
    return _font;
}
