#pragma once

#include <memory>

namespace golias {

    class Texture2D;

    struct Glyph {
        int x0, y0; // bottom-left
        int x1, y1; // top-right

        int width;
        int height;

        int advance; // bearing to advance to next glyph
    };


    class Font {
    public:

        int GetSize() const;

        const Glyph& GetGlyph(char code) const;

        const std::shared_ptr<Texture2D>& GetTexture() const;

        void SetSize(int size);

        void SetGlyphDescription(char code, const Glyph& glyph);

        void SetTexture(const std::shared_ptr<Texture2D>& tex) ;
    private:
        int fontSize = 0;
        Glyph glyphs[128] = {};

        std::shared_ptr<Texture2D> texture;
    };
}; // namespace golias
