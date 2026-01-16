#include "font/font.h"

#include <freetype/freetype.h>

namespace golias {


    int Font::GetSize() const {
        if (face) {
            return face->size->metrics.y_ppem;
        }

        return 0;
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

    void Font::SetFace(FT_Face ftFace) {
        face = ftFace;
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

    void Font::SetSize(int fontSize) {

        if (face) {
            FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(fontSize));
        }
    }

    void Font::SetGlyphDescription(uint32_t codepoint, const Glyph& glyph) {
        glyphs[codepoint] = glyph;
    }

    void Font::SetTexture(const std::shared_ptr<Texture2D>& tex) {
        texture = tex;
    }

    void Font::Destroy() {
        if (face) {
            FT_Done_Face(face);
            face = nullptr;
        }

        glyphs.clear();
        texture = nullptr;
    }

    Font::~Font() = default;
} // namespace golias
