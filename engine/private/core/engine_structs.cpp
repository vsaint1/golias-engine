#include "core/engine_structs.h"

#include "core/ember_core.h"
#include "core/engine.h"

const Color Color::RED         = {255, 0, 0, 255};
const Color Color::GREEN       = {0, 255, 0, 255};
const Color Color::BLUE        = {0, 0, 255, 255};
const Color Color::WHITE       = {255, 255, 255, 255};
const Color Color::BLACK       = {0, 0, 0, 255};
const Color Color::YELLOW      = {255, 255, 0, 255};
const Color Color::CYAN        = {0, 255, 255, 255};
const Color Color::MAGENTA     = {255, 0, 255, 255};
const Color Color::TRANSPARENT = {0, 0, 0, 0};


Texture::~Texture() {

}

Font::~Font() {
}

bool Rect2::is_zero() const {
    return width == 0 && height == 0 && x == 0 && y == 0;
}

glm::vec2 Rect2::top_left() const {
    return {x, y};
}
glm::vec2 Rect2::bottom_right() const {
    return {x + width, y + height};
}

bool Recti::is_zero() const {
    return width == 0 && height == 0 && x == 0 && y == 0;
}

glm::ivec2 Recti::top_left() const {
    return {x, y};
}

glm::ivec2 Recti::bottom_right() const {
    return {x + width, y + height};
}

bool Font::is_valid() const {
    return !characters.empty();
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

Color Color::to_rgba() const {

    return *this;
}

bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

Sizei _calc_text_size(const std::string& text, const Font& font, float font_size) {
    float scale_factor = (font.font_size > 0.0f) ? (font_size / font.font_size) : 1.0f;
    int text_width     = 0;
    int text_height    = 0;

    for (char c : text) {
        if (c == '\n') {
            continue;
        }
        if (font.characters.find(c) == font.characters.end()) {
            continue;
        }

        const Character& g = font.characters.at(c);
        text_height        = SDL_max(text_height, static_cast<int>(g.size.y * scale_factor));
        text_width += static_cast<int>(g.advance * scale_factor);
    }

    return {text_width, text_height};
}
