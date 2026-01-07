#pragma once

#include "font.h"
#include <string>
#include <unordered_map>

typedef struct FT_LibraryRec_* FT_Library;

namespace golias {

    class FontManager {
    public:
        FontManager() = default;
        ~FontManager();

        bool Initialize();

        std::shared_ptr<Font> GetFont(const std::string_view pPath, int size);

    private:
        FT_Library ftLibrary = nullptr;
        using FontFamily     = std::unordered_map<int, std::shared_ptr<Font>>; // Map size to Font
        std::unordered_map<std::string, FontFamily> fonts;
    };
} // namespace golias
