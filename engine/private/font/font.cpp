#include "font/font.h"

namespace golias {


    int Font::GetSize() const {
        return fontSize;
    }

    int Font::GetAscender() const {
        return ascender;
    }

    int Font::GetDescender() const {
        return descender;
    }

    void Font::SetAscender(int asc) {
        ascender = asc;
    }

    void Font::SetDescender(int desc) {
        descender = desc;
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
