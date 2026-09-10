#pragma once

#include "stdafx.h"

namespace golias {

    using Path = std::filesystem::path;

    class FileSystem {

    public:
        FileSystem()  = default;
        ~FileSystem() = default;

        Path GetExecutablePath() const;

        /// @brief  Base folder assets
        Path GetAssetsFolder() const;

        /// @brief  User-level app config directory (e.g. %APPDATA%/Golias on Windows,
        ///         ~/.config/golias on Linux/macOS).
        Path GetUserConfigDir() const;

        bool FileExists(const Path& path) const;

        std::vector<char> LoadFile(const Path& path) const;

        std::vector<char> LoadAssetFile(CString path) const;

        String LoadAssetFileText(CString path) const;

        /// @brief Writes text to an `Writable` path (creates parent directories).
        bool SaveFileText(const Path& path, CString contents) const;

    };
} // namespace golias
