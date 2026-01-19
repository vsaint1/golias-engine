#include "core/asset_manager.h"

#include "core/engine.h"
#include "core/graphics/material.h"
#include "core/graphics/texture_2d.h"
#include "core/graphics/texture_cubemap.h"
#include "font/font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace golias {

    struct UnicodeRange {
        uint32_t from;
        uint32_t to;
    };


    static void AddRange(std::vector<uint32_t>& out, uint32_t from, uint32_t to) {
        out.reserve(out.size() + (to - from + 1));
        for (uint32_t c = from; c <= to; ++c) {
            out.push_back(c);
        }
    }

    static bool IsCJKFont(const std::string& path) {
        static constexpr std::array<std::string_view, 8> keywords = {"CJK", "JP", "KR", "SC", "TC", "Japanese", "Korean", "Chinese"};

        for (auto key : keywords) {
            if (path.find(key) != std::string::npos) {
                return true;
            }
        }

        return false;
    }

    bool AssetManager::Initialize() {
        FT_Error error = FT_Init_FreeType(&ftLibrary);

        if (error != FT_Err_Ok) {
            spdlog::error("AssetManager::Initialize Failed to initialize FreeType Library. Error: {}", FT_Error_String(error));
            return false;
        }

        return true;
    }

    AssetManager::~AssetManager() {

        for (const auto& [path, fontFamily] : fonts) {
            for (const auto& [size, font] : fontFamily) {
                if (font) {
                    font->Destroy();
                }
            }
        }

        if (ftLibrary) {
            FT_Done_FreeType(ftLibrary);
            ftLibrary = nullptr;
        }
    }


    std::shared_ptr<Texture2D> AssetManager::EnsureTexture2D(const std::string_view pFilePath) {


        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            spdlog::debug("Texture cache HIT: {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateTextureFromFile(pFilePath);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug("Texture cache MISS, loaded: {} (Total cached: {})", pFilePath, textures2D.size());
        }

        return texture;
    }

    std::shared_ptr<Texture2D>
        AssetManager::EnsureTexture2D(const std::string_view pFilePath, int width, int height, ETextureFormat format, const Uint8* data) {

        auto it = textures2D.find(pFilePath.data());

        if (it != textures2D.end()) {
            spdlog::debug("Texture cache HIT (embedded): {} (Total cached: {})", pFilePath, textures2D.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateTextureFromData(width, height, format, data);

        if (texture) {
            textures2D[pFilePath.data()] = texture;
            spdlog::debug(
                "Texture cache MISS (embedded), created: {} {}x{} (Total cached: {})", pFilePath, width, height, textures2D.size());
        }

        return texture;
    }

    std::shared_ptr<TextureCubemap> AssetManager::EnsureTextureCubemapCross(const std::string_view pFilePath) {

        auto it = texturesCubemap.find(pFilePath.data());
        if (it != texturesCubemap.end()) {
            spdlog::debug("Cubemap cache HIT (cross): {} (Total cached: {})", pFilePath, texturesCubemap.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateCubemapFromCross(pFilePath.data());

        if (texture) {
            texturesCubemap[pFilePath.data()] = texture;
            spdlog::debug("Cubemap cache MISS (cross), loaded: {} (Total cached: {})", pFilePath, texturesCubemap.size());
        }

        return texture;
    }

    std::shared_ptr<TextureCubemap> AssetManager::EnsureTextureCubemapFaces(const std::array<std::string, 6>& faces) {
        auto it = texturesCubemap.find(faces.back());
        if (it != texturesCubemap.end()) {
            spdlog::debug("Cubemap cache HIT (faces): {} (Total cached: {})", faces[0], texturesCubemap.size());
            return it->second;
        }

        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        auto texture = rd->CreateCubemapFromFaces(faces);

        if (texture) {
            texturesCubemap[faces[0]] = texture;
            spdlog::debug("Cubemap cache MISS (faces), loaded: {} (Total cached: {})", faces[0], texturesCubemap.size());
        }

        return texture;
    }

    std::shared_ptr<TextureCubemap> AssetManager::EnsureTextureCubemapProcedural(const std::string_view pIdentifier) {
        auto it = texturesCubemap.find(pIdentifier.data());
        if (it != texturesCubemap.end()) {
            spdlog::debug("Cubemap cache HIT (procedural): {} (Total cached: {})", pIdentifier, texturesCubemap.size());
            return it->second;
        }

        auto rd      = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
        auto texture = rd->CreateCubemapProcedural();

        if (texture) {
            texturesCubemap[pIdentifier.data()] = texture;
            spdlog::debug("Cubemap cache MISS (procedural), created: {} (Total cached: {})", pIdentifier, texturesCubemap.size());
            return texture;
        }

        spdlog::warn("Cubemap cache MISS (procedural), but no procedural generation implemented: {}", pIdentifier);
        return nullptr;
    }

    std::shared_ptr<Material> AssetManager::EnsureMaterial(const std::string_view pFilePath) {
        auto it = materials.find(pFilePath.data());
        if (it != materials.end()) {
            return it->second;
        }

        return nullptr;
    }


    void AssetManager::RegisterMaterial(const std::string_view pFilePath, const std::shared_ptr<Material>& pMaterial) {
        materials[pFilePath.data()] = pMaterial;
    }

    std::shared_ptr<Font> AssetManager::EnsureFont(const std::string_view pFilePath, int size) {


        if (auto familyIt = fonts.find(pFilePath.data()); familyIt != fonts.end()) {
            if (auto fontIt = familyIt->second.find(size); fontIt != familyIt->second.end()) {
                return fontIt->second;
            }
        }

        std::vector<uint32_t> codepoints;

        // ---- Base Latin + European scripts ----
        static constexpr UnicodeRange baseRanges[] = {
            {0x0020, 0x007F}, // Basic Latin
            {0x00A0, 0x00FF}, // Latin-1 Supplement
            {0x0100, 0x017F}, // Latin Extended-A
            {0x0180, 0x024F}, // Latin Extended-B
            {0x0370, 0x03FF}, // Greek
            {0x0400, 0x04FF}, // Cyrillic
        };

        for (const auto& r : baseRanges) {
            AddRange(codepoints, r.from, r.to);
        }

        // ---- Optional CJK blocks ----
        const std::string pathStr(pFilePath);
        if (IsCJKFont(pathStr)) {
            static constexpr UnicodeRange cjkRanges[] = {
                {0x3040, 0x309F}, // Hiragana
                {0x30A0, 0x30FF}, // Katakana
                {0x3000, 0x303F}, // CJK Symbols & Punctuation
                {0x4E00, 0x9FFF}, // CJK Unified Ideographs
                {0x1100, 0x11FF}, // Hangul Jamo
                {0xAC00, 0xD7A3}, // Hangul Syllables
            };

            for (const auto& r : cjkRanges) {
                AddRange(codepoints, r.from, r.to);
            }

            spdlog::info("AssetManager::EnsureFont Loading CJK font '{}' with {} codepoints", pFilePath, codepoints.size());
        }

        return LoadFont(pFilePath, size, codepoints);
    }




    void AssetManager::SetFallbackFonts(const std::vector<std::string>& fallbackPaths) {
        fallbackFonts = fallbackPaths;
    }

    const Glyph*
        AssetManager::GetGlyphWithFallback(const std::shared_ptr<Font>& primaryFont, uint32_t codepoint, std::shared_ptr<Font>& outFont) {
        if (!primaryFont) {
            return nullptr;
        }

        const Glyph* glyph = primaryFont->GetGlyph(codepoint);
        if (glyph) {
            outFont = primaryFont;
            return glyph;
        }

        for (const auto& fallbackPath : fallbackFonts) {
            auto fallbackFont = EnsureFont(fallbackPath, primaryFont->GetSize());
            if (fallbackFont) {
                glyph = fallbackFont->GetGlyph(codepoint);
                if (glyph) {
                    outFont = fallbackFont;
                    spdlog::debug("AssetManager: Using fallback font '{}' for codepoint U+{:04X}", fallbackPath, codepoint);
                    return glyph;
                }
            }
        }

        return nullptr;
    }

    std::shared_ptr<Font> AssetManager::LoadFont(const std::string_view path, int size, const std::vector<uint32_t>& codepoints) {
        auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);
        if (buffer.empty()) {
            spdlog::error("AssetManager::LoadFont Failed to load font file: {}", path);
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
        Uint8* atlas            = static_cast<Uint8*>(SDL_malloc(totalBytes));

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


            if (penX + static_cast<int>(bmp.width) >= textureWidth) {
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

            Glyph gd;
            gd.x0      = penX;
            gd.y0      = penY;
            gd.x1      = penX + static_cast<int>(bmp.width);
            gd.y1      = penY + static_cast<int>(bmp.rows);
            gd.width   = static_cast<int>(bmp.width);
            gd.height  = static_cast<int>(bmp.rows);
            gd.advance = (face->glyph->advance.x >> 6);
            gd.xOffset = static_cast<int>(face->glyph->bitmap_left);
            gd.yOffset = static_cast<int>(face->glyph->bitmap_top);

            font->SetGlyphDescription(codepoint, gd);

            penX += static_cast<int>(bmp.width + 1);
            loadedGlyphs++;
        }

        auto texture = EnsureTexture2D(path, textureWidth, textureHeight, ETextureFormat::RGBA8, atlas);


        font->SetTexture(texture);
        font->SetSize(size);
        font->SetAscender(face->size->metrics.ascender >> 6);
        font->SetDescender(face->size->metrics.descender >> 6);
        font->SetFace(face);

        fonts[path.data()][size] = font;

        // FT_Done_Face(face);

        spdlog::info("AssetManager::LoadFont Loaded font '{}' (size: {}) with {} glyphs in {}x{} atlas",
                     path,
                     size,
                     loadedGlyphs,
                     textureWidth,
                     textureHeight);

        return font;
    }
} // namespace golias
