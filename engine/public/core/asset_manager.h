#pragma once
#include "core/graphics/structs.h"
#include "font/font.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

typedef struct FT_LibraryRec_* FT_Library;

namespace golias {

    class Texture2D;
    class TextureCubemap;
    class Material;
    class Font;





    class AssetManager {
    public:
        ~AssetManager();

        bool Initialize();

        std::shared_ptr<Texture2D> EnsureTexture2D(const std::string_view pFilePath);
        std::shared_ptr<Texture2D>
            EnsureTexture2D(const std::string_view pFilePath, int width, int height, ETextureFormat format, const Uint8* data);

        std::shared_ptr<TextureCubemap> EnsureTextureCubemapCross(const std::string_view pFilePath);
        std::shared_ptr<TextureCubemap> EnsureTextureCubemapFaces(const std::array<std::string, 6>& faces);
        std::shared_ptr<TextureCubemap> EnsureTextureCubemapProcedural(const std::string_view pIdentifier = "procedural_cubemap");

        std::shared_ptr<Material> EnsureMaterial(const std::string_view pFilePath);
        void RegisterMaterial(const std::string_view pFilePath, const std::shared_ptr<Material>& pMaterial);

        std::shared_ptr<Font> EnsureFont(const std::string_view pFilePath, int size);

        
        // Set fallback fonts for when a glyph is not found in the primary font
        void SetFallbackFonts(const std::vector<std::string>& fallbackPaths);

        // Get glyph from primary font or fallback fonts
        const Glyph* GetGlyphWithFallback(const std::shared_ptr<Font>& primaryFont, uint32_t codepoint, std::shared_ptr<Font>& outFont);

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures2D;
        std::unordered_map<std::string, std::shared_ptr<TextureCubemap>> texturesCubemap;
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;

    private:
        std::shared_ptr<Font> LoadFont(const std::string_view path, int size, const std::vector<uint32_t>& codepoints);

        FT_Library ftLibrary = nullptr;
        using FontFamily     = std::unordered_map<int, std::shared_ptr<Font>>; // Map size to Font
        std::unordered_map<std::string, FontFamily> fonts;
        std::vector<std::string> fallbackFonts;
    };
} // namespace golias
