#include "core/engine_structs.h"



bool Font::IsValid() const {
    return texture.id != 0 && glyphs.size() > 0;
}

int Font::GetTextWidth(const std::string& text, float size) const {

    float scale_factor = (font_size > 0.0f) ? (size / font_size) : 1.0f;
    int text_width     = 0;

    for (char c : text) {
        if (c == '\n') {
            continue;
        }

        if (glyphs.find(c) == glyphs.end()) {
            continue;
        }

        const Glyph& g = glyphs.at(c);

        text_width += (g.advance) * scale_factor;
    }

    return text_width;
}

glm::vec4 Color::GetNormalizedColor() const {
    glm::vec4 norm_color = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f,
    };

    return norm_color;
}
