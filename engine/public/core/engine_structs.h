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

// Windows API conflict
namespace ember {

    struct Rectangle {
        int x;
        int y;
        int width;
        int height;
    };

    struct Size {
        int width;
        int height;
    };

}; // namespace ember

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

    glm::vec4 normalize_color() const;

    bool operator==(const Color& other) const;
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
    int font_size;
    float kerning = 0.0f;
    int ascent, descent, line_gap;
    float scale;

    bool is_valid() const;

    Font() = default;
};


ember::Size _calc_text_size(const std::string& text, const Font& font, float font_size = 0.0f);
