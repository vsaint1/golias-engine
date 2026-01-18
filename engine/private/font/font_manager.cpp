#include "font/font_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/engine.h"
#include <set>

namespace golias {

    FontManager::~FontManager() {

        for(const auto& [path, fontFamily] : fonts){
            for(const auto& [size, font] : fontFamily){
                if(font){
                    font->Destroy();
                }
            }
        }

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

    void FontManager::SetFallbackFonts(const std::vector<std::string>& fallbackPaths) {
        fallbackFonts = fallbackPaths;
    }

    const Glyph*
        FontManager::GetGlyphWithFallback(const std::shared_ptr<Font>& primaryFont, uint32_t codepoint, std::shared_ptr<Font>& outFont) {
        if (!primaryFont) {
            return nullptr;
        }

        const Glyph* glyph = primaryFont->GetGlyph(codepoint);
        if (glyph) {
            outFont = primaryFont;
            return glyph;
        }

        for (const auto& fallbackPath : fallbackFonts) {
            auto fallbackFont = GetFont(fallbackPath, primaryFont->GetSize());
            if (fallbackFont) {
                glyph = fallbackFont->GetGlyph(codepoint);
                if (glyph) {
                    outFont = fallbackFont;
                    spdlog::debug("FontManager: Using fallback font '{}' for codepoint U+{:04X}", fallbackPath, codepoint);
                    return glyph;
                }
            }
        }

        return nullptr;
    }

    std::shared_ptr<Font> FontManager::GetFont(const std::string_view path, int size) {
        auto fontFamilyIt = fonts.find(path.data());
        if (fontFamilyIt != fonts.end()) {
            auto fontIt = fontFamilyIt->second.find(size);
            if (fontIt != fontFamilyIt->second.end()) {
                return fontIt->second;
            }
        }

        std::vector<uint32_t> codepoints;

        // Basic Latin (ASCII)
        for (uint32_t c = 0x0020; c <= 0x007F; ++c) {
            codepoints.push_back(c);
        }

        // Latin-1 Supplement
        for (uint32_t c = 0x00A0; c <= 0x00FF; ++c) {
            codepoints.push_back(c);
        }

        // Latin Extended-A
        for (uint32_t c = 0x0100; c <= 0x017F; ++c) {
            codepoints.push_back(c);
        }

        // Latin Extended-B
        for (uint32_t c = 0x0180; c <= 0x024F; ++c) {
            codepoints.push_back(c);
        }

        // Greek and Coptic
        for (uint32_t c = 0x0370; c <= 0x03FF; ++c) {
            codepoints.push_back(c);
        }

        // Cyrillic
        for (uint32_t c = 0x0400; c <= 0x04FF; ++c) {
            codepoints.push_back(c);
        }

        std::string pathStr(path);
        bool isCJKFont = (pathStr.find("CJK") != std::string::npos) || (pathStr.find("JP") != std::string::npos)
                      || (pathStr.find("KR") != std::string::npos) || (pathStr.find("SC") != std::string::npos)
                      || (pathStr.find("TC") != std::string::npos) || (pathStr.find("Japanese") != std::string::npos)
                      || (pathStr.find("Korean") != std::string::npos) || (pathStr.find("Chinese") != std::string::npos);

        if (isCJKFont) {
            // Hiragana
            for (uint32_t c = 0x3040; c <= 0x309F; ++c) {
                codepoints.push_back(c);
            }

            // Katakana
            for (uint32_t c = 0x30A0; c <= 0x30FF; ++c) {
                codepoints.push_back(c);
            }

            // CJK Unified Ideographs (Common Kanji/Hanzi) - most frequently used
            // Loading all 20k+ would be too much, so we load the most common ranges
            for (uint32_t c = 0x4E00; c <= 0x9FFF; ++c) {
                codepoints.push_back(c);
            }

            // Hangul Syllables (Korean)
            for (uint32_t c = 0xAC00; c <= 0xD7A3; ++c) {
                codepoints.push_back(c);
            }

            // Hangul Jamo
            for (uint32_t c = 0x1100; c <= 0x11FF; ++c) {
                codepoints.push_back(c);
            }

            // CJK Symbols and Punctuation
            for (uint32_t c = 0x3000; c <= 0x303F; ++c) {
                codepoints.push_back(c);
            }

            spdlog::info("FontManager::GetFont Loading CJK font '{}' with {} codepoints", path, codepoints.size());
        }

        return LoadFont(path, size, codepoints);
    }

    std::shared_ptr<Font> FontManager::LoadFont(const std::string_view path, int size, const std::vector<uint32_t>& codepoints) {
        auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);
        if (buffer.empty()) {
            spdlog::error("FontManager::LoadFont Failed to load font file: {}", path);
            return nullptr;
        }

        FT_Face face;
        FT_Error result = FT_New_Memory_Face(ftLibrary, reinterpret_cast<FT_Byte*>(buffer.data()), buffer.size(), 0, &face);
        if (result != FT_Err_Ok) {
            spdlog::error("FontManager::LoadFont Failed to create face for font: {}", path);
            return nullptr;
        }

        FT_Set_Pixel_Sizes(face, 0, size);

        const int lineHeight = face->size->metrics.height >> 6;

        int estimatedSize = static_cast<int>(std::sqrt(static_cast<float>(codepoints.size())) * (lineHeight + 1));
        int textureWidth  = 1024;

        int maxSize = (codepoints.size() > 10000) ? 8192 : 4096;

        while (textureWidth < estimatedSize && textureWidth < maxSize) {
            textureWidth <<= 1;
        }

        int textureHeight = textureWidth;

        const size_t stride     = textureWidth * 4;
        const size_t totalBytes = static_cast<size_t>(textureWidth * textureHeight * 4);
        Uint8* atlas = static_cast<Uint8*>(SDL_malloc(totalBytes));

        SDL_memset(atlas, 0, totalBytes);

        int penX = 0;
        int penY = 0;

        auto font        = std::make_shared<Font>();
        int loadedGlyphs = 0;

        for (uint32_t codepoint : codepoints) {
            FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
            if (glyph_index == 0) {
                continue;
            }

            if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) != FT_Err_Ok) {
                continue;
            }

            FT_Bitmap& bmp = face->glyph->bitmap;

          
            if (penX + static_cast<int>(bmp.width) >= textureWidth)
            {
                penX = 0;
                penY += lineHeight + 1;
            }
            
            for (uint32_t row = 0; row < bmp.rows; ++row) {
                for (uint32_t col = 0; col < bmp.width; ++col) {
                    int x = penX + static_cast<int>(col);
                    int y = penY + static_cast<int>(row); 

                    if (x < 0 || x >= textureWidth || y < 0 || y >= textureHeight) {
                        continue;
                    }

                    const unsigned char value = bmp.buffer[row * bmp.pitch + col];
                    const size_t idx          = static_cast<size_t>(y) * stride + x * 4;

                    atlas[idx + 0] = value;
                    atlas[idx + 1] = value;
                    atlas[idx + 2] = value;
                    atlas[idx + 3] = value;
                }
            }

            Glyph  gd;
            gd.x0 = penX;
            gd.y0 = penY;
            gd.x1 = penX + static_cast<int>(bmp.width);
            gd.y1 = penY + static_cast<int>(bmp.rows);
            gd.width = static_cast<int>(bmp.width);
            gd.height = static_cast<int>(bmp.rows);
            gd.advance = (face->glyph->advance.x >> 6);
            gd.xOffset = static_cast<int>(face->glyph->bitmap_left);
            gd.yOffset = static_cast<int>(face->glyph->bitmap_top);

            font->SetGlyphDescription(codepoint, gd);

            penX += static_cast<int>(bmp.width + 1);
            loadedGlyphs++;
        }

        auto texture =
            Engine::GetInstance().GetTextureManager().EnsureTexture2D(path, textureWidth, textureHeight, ETextureFormat::RGBA8, atlas);

        
        font->SetTexture(texture);
        font->SetSize(size);
        font->SetAscender(face->size->metrics.ascender >> 6);
        font->SetDescender(face->size->metrics.descender >> 6);
        font->SetFace(face);

        fonts[path.data()][size] = font;

        // FT_Done_Face(face);

        spdlog::info("FontManager::LoadFont Loaded font '{}' (size: {}) with {} glyphs in {}x{} atlas",
                     path,
                     size,
                     loadedGlyphs,
                     textureWidth,
                     textureHeight);

        return font;
    }

} // namespace golias
