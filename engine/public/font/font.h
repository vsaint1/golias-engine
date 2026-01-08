#pragma once

#include <memory>
#include <unordered_map>

namespace golias {

    class Texture2D;

    struct Glyph {
        int x0, y0; // bottom-left
        int x1, y1; // top-right

        int width;
        int height;

        int advance; // bearing to advance to next glyph
        int xOffset = 0;
        int yOffset = 0;
    };


    class Font {
    public:

        int GetSize() const;

        const Glyph* GetGlyph(uint32_t codepoint) const;
        bool HasGlyph(uint32_t codepoint) const;
        void SetGlyphDescription(uint32_t codepoint, const Glyph& glyph);

        const std::shared_ptr<Texture2D>& GetTexture() const;

        void SetSize(int size);

        void SetTexture(const std::shared_ptr<Texture2D>& tex);

        int GetAscender() const{
            return ascender;
        }

        int GetDescender() const{
            return descender;
        }

        void SetAscender(int asc){
            ascender = asc;
        }

        void SetDescender(int desc){
            descender = desc;
        }
    private:
        int fontSize = 0;
        int ascender  = 0;
        int descender = 0;
        std::unordered_map<uint32_t, Glyph> glyphs;

        std::shared_ptr<Texture2D> texture;
    };
}; // namespace golias
