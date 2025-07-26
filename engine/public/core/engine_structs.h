#pragma once

#include "imports.h"
#include <freetype/freetype.h>
#include <freetype/ftstroke.h>


// Core struct (engine stuff) - 0.0.1

/*!
 *  @brief Texture base class
 *
 *  @version 0.0.1
 */
typedef struct Texture {
    unsigned int id = 0;
    int width       = 0;
    int height      = 0;


    Texture() = default;
} Texture, Texture2D;


struct Rect2 {
    float x;
    float y;
    float width;
    float height;
};

struct Recti {
    int x;
    int y;
    int width;
    int height;
};

struct Size {
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

/*!
 *  @brief Font struct
 *
 *  @details TTF fonts
 *
 *  @version 0.0.1
 */
struct Font {
    HashMap<char, Glyph> glyphs{};
    Texture texture;
    int font_size = 16;
    float kerning = 0.0f;
    int ascent = 0, descent = 0, line_gap = 0;
    float scale = 1.0f;

    [[nodiscard]] bool is_valid() const;

    Font() = default;
};


Size calc_text_size(const std::string& text, const Font& font, float font_size = 0.0f);
