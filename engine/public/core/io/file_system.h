#pragma once

#include "stdafx.h"

namespace golias {

    using Path = std::filesystem::path;

    class FileSystem {

    public:
        FileSystem() = default;
        ~FileSystem() = default;

        Path GetExecutablePath() const;

        Path GetAssetsFolder() const;

        bool FileExists(const Path& path) const;

        std::vector<char> LoadFile(const Path& path) const;

        std::vector<char> LoadAssetFile(CString path) const;

        String LoadAssetFileText(CString path) const;


    };
} // namespace golias
