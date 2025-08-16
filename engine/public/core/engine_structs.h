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

    ~Texture();

    void* mtlTexture = nullptr; // id<MTLTexture>

};


struct Rect2 {
    float x, y, width, height;

    Rect2() : x(0), y(0), width(0), height(0) {}

    Rect2(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}

    [[nodiscard]] bool is_zero() const;

    [[nodiscard]] glm::vec2 top_left() const;

    [[nodiscard]] glm::vec2 bottom_right() const ;

};

struct Recti {
    int x, y, width, height;

    Recti() : x(0), y(0), width(0), height(0) {}

    Recti(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

    [[nodiscard]] bool is_zero() const;

    [[nodiscard]] glm::ivec2 top_left() const;

    [[nodiscard]] glm::ivec2 bottom_right() const ;
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

    [[nodiscard]] Color to_rgba() const;

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


struct Character {
    GLuint texture_id;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

/*!
 *  @brief Glyph struct
 *
 *  @version 0.0.8
 */
struct Glyph {
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
    HashMap<char, Character> characters;
    std::string font_path;
    int font_size = 48;
    float kerning = 0.0f;
    int ascent = 0, descent = 0, line_gap = 0;
    float scale = 1.0f;

    [[nodiscard]] bool is_valid() const;

    Font() = default;
    ~Font();
};


Sizei calc_text_size(const std::string& text, const Font& font, float font_size = 0.0f);
