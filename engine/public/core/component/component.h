#pragma once

#include "imports.h"
#include <stb_truetype.h>

// TODO: ( REFACTOR THIS ) `JUST FOR CODE ORGANIZATION, SOME ARE NOT COMPONENTS`


typedef struct Texture {
    unsigned int id = 0;
    int width       = 0;
    int height      = 0;
} Texture, Texture2D;

// Windows API conflict
namespace ember {

    struct Rectangle {
        int x;
        int y;
        int width;
        int height;
    };
    
}; // namespace ember

struct Audio {
    ma_decoder* decoder;
    ma_sound sound;
    float volume = 1.0f;
    float duration = 0.0f;
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};


struct Glyph {
    float x0, y0, x1, y1;
    int w, h;
    int x_offset, y_offset;
    int advance;
};

struct Font {
    std::map<char, Glyph> glyphs;
    Texture texture;
    int font_size;
    float kerning = 0.0f;
    int ascent, descent, line_gap;
    int scale;

    bool IsValid() const;
};
