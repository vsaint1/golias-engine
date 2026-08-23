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

    };
} // namespace golias
