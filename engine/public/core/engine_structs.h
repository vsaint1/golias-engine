#pragma once

#include "imports.h"



// Core struct (engine stuff) - 0.0.1

/*!
 *  @brief Texture base class
 *
 *  @version 0.0.1
 */
struct Texture {
    Uint32 id  = 0;
    int width  = 0;
    int height = 0;
    std::string path;

    Texture()                          = default;

    
    void* mtlTexture = nullptr; // id<MTLTexture>

};


struct Rect2 {
    float x, y, width, height;

    Rect2(float x = 0, float y = 0, float width = 0, float height = 0) : x(x), y(y), width(width), height(height) {
    }

    bool is_zero() const {
        return width == 0 && height == 0 && x == 0 && y == 0;
    }

    glm::vec2 top_left() const {
        return {x, y};
    }
    glm::vec2 bottom_right() const {
        return {x + width, y + height};
    }
};

struct Recti {
    int x;
    int y;
    int width;
    int height;
};

struct Sizei {
    int width;
    int height;
};


/*!
 *  @brief Color struct
 *
 *  @details Works with HSV, RGBA, RGB and Normalized colors (0-1)
 *
 *  @version 0.0.1
 */

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    [[nodiscard]] glm::vec4 normalize_color() const;

    bool operator==(const Color& other) const;

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color WHITE;
    static const Color BLACK;
    static const Color YELLOW;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color TRANSPARENT;
};

/*!
 *  @brief Glyph struct
 *
 *  @version 0.0.1
 */
struct Glyph {
    float x0, y0, x1, y1;
    int w, h;
    int x_offset, y_offset;
    int advance;
};



struct Character {
    GLuint texture_id;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

struct CharData {
    float x0, y0, w, h;
    const Character* ch;
    const glm::vec4* token_color;
};
/*!
 *  @brief Font struct
 *
 *  @details TTF fonts
 *
 *  @version 0.0.1
 */
struct Font {
    std::unordered_map<char, Character> characters;
    std::string font_path;
    int font_size = 48;
    float kerning = 0.0f;
    int ascent = 0, descent = 0, line_gap = 0;
    float scale = 1.0f;

    [[nodiscard]] bool is_valid() const;

    Font() = default;
};


Sizei calc_text_size(const std::string& text, const Font& font, float font_size = 0.0f);
