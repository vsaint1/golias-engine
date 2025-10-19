#include "core/renderer/base_struct.h"


Font::~Font() {
    if (_font) {
        TTF_CloseFont(_font);
        _font = nullptr;
    }
}

glm::vec2 Font::get_size(const std::string& text) {
    int w = 0, h = 0;

    if (_font && TTF_GetStringSize(_font, text.c_str(), text.size(), &w, &h) == 0) {
        return {float(w), float(h)};
    }

    return {0, 0};
}

TTF_Font* Font::get_font() const {
    return _font;
}


bool Material::is_valid() const {
    const bool albedo_valid = albedo_texture != nullptr && albedo_texture->is_valid();

    const bool validated = albedo_valid; // && normal_valid && normal_map && etc.
    return validated;
}

void Material::bind() {

    if (albedo_texture && albedo_texture->is_valid()) {
        albedo_texture->bind();
    }

    if (metallic.texture && metallic.texture->is_valid()) {
        metallic.texture->bind(1);
    }
}
