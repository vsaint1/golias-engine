#include "font/font.h"

namespace golias {


    int Font::GetSize() const {
        return fontSize;
    }

    const Glyph* Font::GetGlyph(uint32_t codepoint) const {
        auto it = glyphs.find(codepoint);
        if (it != glyphs.end()) {
            return &it->second;
        }
        return nullptr;
    }

    bool Font::HasGlyph(uint32_t codepoint) const {
        return glyphs.find(codepoint) != glyphs.end();
    }

    const std::shared_ptr<Texture2D>& Font::GetTexture() const {
        return texture;
    }

    void Font::SetSize(int size) {
        fontSize = size;
    }

    void Font::SetGlyphDescription(uint32_t codepoint, const Glyph& glyph) {
        glyphs[codepoint] = glyph;
    }

    void Font::SetTexture(const std::shared_ptr<Texture2D>& tex) {
        texture = tex;
    }
} // namespace golias
