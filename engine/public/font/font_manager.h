#pragma once

#include "font.h"
#include <string>
#include <unordered_map>
#include <vector>

typedef struct FT_LibraryRec_* FT_Library;

namespace golias {

    class FontManager {
    public:
        FontManager() = default;
        ~FontManager();

        bool Initialize();

        std::shared_ptr<Font> GetFont(const std::string_view pPath, int size);
        
        // Set fallback fonts for when a glyph is not found in the primary font
        void SetFallbackFonts(const std::vector<std::string>& fallbackPaths);
        
        // Get glyph from primary font or fallback fonts
        const Glyph* GetGlyphWithFallback(const std::shared_ptr<Font>& primaryFont, uint32_t codepoint, std::shared_ptr<Font>& outFont);

    private:
        std::shared_ptr<Font> LoadFont(const std::string_view path, int size, const std::vector<uint32_t>& codepoints);
        
        FT_Library ftLibrary = nullptr;
        using FontFamily     = std::unordered_map<int, std::shared_ptr<Font>>; // Map size to Font
        std::unordered_map<std::string, FontFamily> fonts;
        std::vector<std::string> fallbackFonts;
    };
} // namespace golias
