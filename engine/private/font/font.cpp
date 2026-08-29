#include "font/font.h"

#include "core/engine.h"
#include "graphics/texture_2d.h"
#include <stb_truetype.h>

namespace golias {

    namespace {
        constexpr int kFirstCodepoint = 0;
        constexpr int kCodepointCount = 128; // ASCII

        constexpr int kAtlasWidth  = 1024;
        constexpr int kAtlasHeight = 1024;

        constexpr int kColumns     = 16;
        constexpr int kRows        = 8;
        constexpr int kCellSize    = kAtlasWidth / kColumns;
        constexpr int kCellPadding = 2;

        constexpr int kGlyphMaxSize = kCellSize - kCellPadding * 2;
    } // namespace

    int Font::GetSize() const {
        return mSize;
    }

    const GlyphDesc& Font::GetGlyphDescription(char c) const {
        return mGlyphs[static_cast<unsigned char>(c)];
    }

    int Font::GetAdvance(char c) const {
        return mGlyphs[static_cast<unsigned char>(c)].Advance;
    }

    const Ref<Texture> Font::GetTexture() const {
        return mTexture;
    }


    Ref<Font> Font::Load(CString path, int size) {
        if (path.empty() || size <= 0) {
            GOLIAS_LOG_ERROR("Invalid font request. Path: '%s', size: %d", path.data(), size);
            return nullptr;
        }

        FileSystem& fileSystem = Engine::GetInstance().GetFileSystem();

        const std::vector<char> fileData = fileSystem.LoadAssetFile(path);

        if (fileData.empty()) {
            GOLIAS_LOG_ERROR("Font file is empty or could not be read: %s", path.data());
            return nullptr;
        }

        const auto* fontData = reinterpret_cast<const unsigned char*>(fileData.data());

        const int fontOffset = stbtt_GetFontOffsetForIndex(fontData, 0);

        if (fontOffset < 0) {
            GOLIAS_LOG_ERROR("Font does not contain a usable face: %s", path.data());
            return nullptr;
        }

        stbtt_fontinfo fontInfo{};

        if (!stbtt_InitFont(&fontInfo, fontData, fontOffset)) {
            GOLIAS_LOG_ERROR("Failed to initialize font '%s'", path.data());
            return nullptr;
        }

        const float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(size));

        constexpr size_t atlasPixelCount = static_cast<size_t>(kAtlasWidth) * static_cast<size_t>(kAtlasHeight);

        // Grayscale glyph atlas.
        std::vector<unsigned char> glyphAtlas(atlasPixelCount, 0);

        Ref<Font> font = std::make_shared<Font>();
        font->mSize    = size;

        for (int index = 0; index < kCodepointCount; ++index) {
            const int codepoint = kFirstCodepoint + index;

            GlyphDesc& glyph = font->mGlyphs[index];

            // Metrics.
            int advanceWidth    = 0;
            int leftSideBearing = 0;

            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advanceWidth, &leftSideBearing);

            glyph.Advance = static_cast<int>(std::lround(advanceWidth * scale));

            // Rasterize glyph.
            int bitmapWidth   = 0;
            int bitmapHeight  = 0;
            int bitmapOffsetX = 0;
            int bitmapOffsetY = 0;

            unsigned char* bitmap =
                stbtt_GetCodepointBitmap(&fontInfo, scale, scale, codepoint, &bitmapWidth, &bitmapHeight, &bitmapOffsetX, &bitmapOffsetY);

            if (!bitmap || bitmapWidth <= 0 || bitmapHeight <= 0) {
                if (bitmap) {
                    stbtt_FreeBitmap(bitmap, nullptr);
                }

                continue;
            }

            if (bitmapWidth > kGlyphMaxSize || bitmapHeight > kGlyphMaxSize) {

                GOLIAS_LOG_WARN("Glyph %d ('%c') at size %d does not fit in the atlas cell "
                                "(%dx%d) and was skipped",
                                codepoint,
                                codepoint >= 32 && codepoint < 127 ? static_cast<char>(codepoint) : '?',
                                size,
                                kGlyphMaxSize,
                                kGlyphMaxSize);

                stbtt_FreeBitmap(bitmap, nullptr);
                continue;
            }

            const int cellX = (index % kColumns) * kCellSize;

            const int cellY = (index / kColumns) * kCellSize;

            const int glyphX = cellX + kCellPadding;

            const int glyphY = cellY + kCellPadding;

            // Copy each glyph scanline directly into the atlas.
            //
            // Source stride:
            //     bitmapWidth
            //
            // Destination stride:
            //     kAtlasWidth
            for (int y = 0; y < bitmapHeight; ++y) {
                unsigned char* dst = glyphAtlas.data() + static_cast<size_t>(glyphY + y) * kAtlasWidth + glyphX;

                const unsigned char* src = bitmap + static_cast<size_t>(y) * bitmapWidth;

                std::memcpy(dst, src, static_cast<size_t>(bitmapWidth));
            }

            glyph.X0 = glyphX;
            glyph.Y0 = glyphY;
            glyph.X1 = glyphX + bitmapWidth;
            glyph.Y1 = glyphY + bitmapHeight;

            glyph.Width  = bitmapWidth;
            glyph.Height = bitmapHeight;

            glyph.OffsetX = bitmapOffsetX;
            glyph.OffsetY = bitmapOffsetY;

            stbtt_FreeBitmap(bitmap, nullptr);
        }

        // Convert the grayscale atlas into RGBA.
        //
        // Texture format:
        //     R = 255
        //     G = 255
        //     B = 255
        //     A = glyph coverage
        std::vector<unsigned char> atlas(atlasPixelCount * 4);

        unsigned char* dst       = atlas.data();
        const unsigned char* src = glyphAtlas.data();

        for (size_t i = 0; i < atlasPixelCount; ++i) {
            dst[0] = 255;
            dst[1] = 255;
            dst[2] = 255;
            dst[3] = src[i];

            dst += 4;
        }

        font->mTexture = std::make_shared<Texture2D>(kAtlasWidth, kAtlasHeight, 4, atlas.data());

        return font;
    }

} // namespace golias
