#pragma once

#include "stdafx.h"


namespace golias {

    class FileSystem {
    public:
        FileSystem() = default;

        std::string GetExecutablePath() const;

        std::string GetAssetsPath() const;

        bool FileExists(const std::string& path) const;

        bool CreateDirectory(const std::string& path) const;

        std::string GetAssetFile(const std::string_view pFileName) const;

        std::vector<char> LoadFile(const std::string_view pPath) const;
        std::vector<char> LoadAssetFile(const std::string_view pFileName) const;

        std::string LoadFileText(const std::string_view pPath) const;
        std::string LoadAssetFileText(const std::string_view pFileName) const;

        static std::string  GetFileExtension(const std::string_view pFileName);

    };
} // namespace golias
