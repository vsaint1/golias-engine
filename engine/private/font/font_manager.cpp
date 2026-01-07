#include "font/font_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/engine.h"

namespace golias {

    FontManager::~FontManager() {
        if (ftLibrary) {
            FT_Done_FreeType(ftLibrary);
            ftLibrary = nullptr;
        }
    }

    bool FontManager::Initialize() {
        FT_Error error = FT_Init_FreeType(&ftLibrary);

        if (error != FT_Err_Ok) {
            spdlog::error("FontManager::Initialize Failed to initialize FreeType Library. Error: {}", FT_Error_String(error));
            return false;
        }

        return true;
    }

    std::shared_ptr<Font> FontManager::GetFont(const std::string_view pPath, int size) {
     
        auto it = fonts.find(pPath.data());
        if (it != fonts.end()) {
            auto fontFamilyIt = it->second.find(size);

            if (fontFamilyIt != it->second.end()) {
                return fontFamilyIt->second;
            }
        }

        auto ftBuffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(pPath);
        if (ftBuffer.empty()) {
            spdlog::error("FontManager::GetFont Failed to load font file: {}", pPath);
            return nullptr;
        }

        FT_Face face;
        FT_Error error = FT_New_Memory_Face(
            ftLibrary, reinterpret_cast<const FT_Byte*>(ftBuffer.data()), static_cast<FT_Long>(ftBuffer.size()), 0, &face);

        if (error != FT_Err_Ok) {
            spdlog::error("FontManager::GetFont Failed to create FreeType Face from memory.");
            return nullptr;
        }

        FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(size));

        const int lineHeight = face->size->metrics.height >> 6; // Convert from 26.6 fixed point to integer
        int maxDimension = static_cast<int>(std::sqrt(128.0f) * (lineHeight + 1));


        int textureWidth = 1;
        while (textureWidth < maxDimension) {
            textureWidth <<= 1;
        }

        int textureHeight = textureWidth;

        constexpr int NUM_CHANNELS = 4; // RGBA
        const size_t stride        = textureWidth * NUM_CHANNELS;
        const size_t totalBytes    = static_cast<size_t>(textureWidth * textureHeight * NUM_CHANNELS);

        Uint8* atlas = new Uint8[totalBytes];
        SDL_memset(atlas, 0, totalBytes);

        int penX = 0;
        int penY = 0;

        auto font = std::make_shared<Font>();
        for (char c = 0; c < 128; ++c) {

            if (FT_Load_Char(face, c, FT_LOAD_RENDER) != FT_Err_Ok) {
                Glyph glyph = {0, 0, 0, 0, 0, 0, 0};
                font->SetGlyphDescription(c, glyph);
                continue;
            }

            FT_Bitmap& bitmap = face->glyph->bitmap;

            if (penX + static_cast<int>(bitmap.width) >= textureWidth) {
                penX = 0;
                penY += lineHeight + 1;
            }

            for (unsigned int row = 0; row < bitmap.rows; ++row) {
                for (unsigned int col = 0; col < bitmap.width; ++col) {

                    int x = penX + static_cast<int>(col);
                    int y = penY + static_cast<int>(row);

                    bool isInvalid = (x < 0 || x >= textureWidth || y < 0 || y >= textureHeight) ? true : false;

                    if (isInvalid) {
                        spdlog::debug("FontManager::GetFont Skipping glyph pixel for char '{}' at ({},{}) - out of bounds", c, x, y);
                        continue;
                    }

                    Uint8 value = bitmap.buffer[row * bitmap.pitch + col];

                    const size_t index = static_cast<size_t>(y * stride + x * NUM_CHANNELS);

                    atlas[index + 0] = 255; // R
                    atlas[index + 1] = 255; // G
                    atlas[index + 2] = 255; // B
                    atlas[index + 3] = value; // A
                }
            }

            Glyph glyph;
            glyph.x0      = penX;
            glyph.y0      = penY;
            glyph.x1      = penX + static_cast<int>(bitmap.width);
            glyph.y1      = penY + static_cast<int>(bitmap.rows);
            glyph.width   = static_cast<int>(bitmap.width);
            glyph.height  = static_cast<int>(bitmap.rows);
            glyph.advance = static_cast<int>(face->glyph->advance.x >> 6);

            font->SetGlyphDescription(c, glyph);

            penX += static_cast<int>(bitmap.width) + 1;
        }

        auto texture =
            Engine::GetInstance().GetTextureManager().EnsureTexture2D(pPath, textureWidth, textureHeight, ETextureFormat::RGBA, atlas);

        font->SetTexture(texture);
        font->SetSize(size);

        fonts[std::string(pPath)][size] = font;


        delete[] atlas;
        
        FT_Done_Face(face);

        spdlog::info("FontManager::GetFont Loaded font '{}' with size {} successfully.", pPath, size);
        return font;
    }

} // namespace golias
