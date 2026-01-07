#include "font/font.h"

namespace golias {


    int Font::GetSize() const {
        return fontSize;
    }

    const Glyph& Font::GetGlyph(char code) const {
        return glyphs[static_cast<unsigned char>(code)];
    }

    const std::shared_ptr<Texture2D>& Font::GetTexture() const {
        return texture;
    }

    void Font::SetSize(int size) {
        fontSize = size;
    }

    void Font::SetGlyphDescription(char code, const Glyph& glyph) {
        glyphs[static_cast<unsigned char>(code)] = glyph;
    }

    void Font::SetTexture(const std::shared_ptr<Texture2D>& tex) {
        texture = tex;
    }
} // namespace golias
