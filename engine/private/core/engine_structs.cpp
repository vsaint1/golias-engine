#include "core/engine_structs.h"


bool Font::is_valid() const {
    return texture.id != 0 && glyphs.size() > 0;
}

glm::vec4 Color::normalize_color() const {
    const glm::vec4 norm_color = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f,
    };

    return norm_color;
}

bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

ember::Size CalcTextSize(const std::string& text, const Font& font, float font_size) {
    float scale_factor = (font.font_size > 0.0f) ? (font_size / font.font_size) : 1.0f;
    int text_width     = 0;
    int text_height    = 0;

    for (char c : text) {
        if (c == '\n') {
            continue;
        }
        if (font.glyphs.find(c) == font.glyphs.end()) {
            continue;
        }

        const Glyph& g = font.glyphs.at(c);
        text_height    = SDL_max(text_height, (g.y1 - g.y0) * scale_factor);
        text_width += (g.advance) * scale_factor;
    }

    return {text_width, text_height};
}
