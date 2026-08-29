#pragma once

#include "graphics/texture.h"

namespace golias {

    /// @brief  Pixel rect of a glyph inside the baked atlas texture.
    struct GlyphDesc {
        int X0 = 0; ///< Left edge of the glyph bitmap in the atlas.
        int Y0 = 0; ///< Top edge of the glyph bitmap in the atlas.
        int X1 = 0; ///< Right edge of the glyph bitmap in the atlas.
        int Y1 = 0; ///< Bottom edge of the glyph bitmap in the atlas.

        int Width  = 0; ///< Glyph bitmap width in pixels.
        int Height = 0; ///< Glyph bitmap height in pixels.

        int OffsetX = 0; ///< Baseline-relative left bearing (scaled).
        int OffsetY = 0; ///< Baseline-relative top bearing (scaled, negative).

        int Advance = 0; ///< Baseline-relative horizontal advance (scaled).
    };

    class Font {
    public:
        static Ref<Font> Load(CString path, int size);

        int GetSize() const;

        const GlyphDesc& GetGlyphDescription(char c) const;

        int GetAdvance(char c) const;

        const Ref<Texture> GetTexture() const;

    private:
        int mSize              = 0;
        GlyphDesc mGlyphs[128] = {};

        Ref<Texture> mTexture = nullptr;
    };

    /// @brief Baked font sizes for a single font family, keyed by pixel size.
    using FontFamily = std::unordered_map<int, Ref<Font>>;

} // namespace golias
