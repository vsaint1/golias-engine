#include "core/io/file_system.h"

#include <cstdlib>
#include <ctime>

namespace golias {

    const char* kAssetsFolderName = "Assets";

    Path FileSystem::GetExecutablePath() const {
        return std::filesystem::current_path();
    }

    Path FileSystem::GetAssetsFolder() const {
 
        return GetExecutablePath() / kAssetsFolderName;
    }

    // TODO: Move this logic to platforms/<platform_name>/file_system.cpp
    Path FileSystem::GetUserConfigDir() const {
#if defined(GOLIAS_PLATFORM_WINDOWS)
        if (const char* appData = std::getenv("APPDATA")) {
            Path dir = Path(appData) / "Golias";
            std::filesystem::create_directories(dir);
            return dir;
        }
#else
        if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
            Path dir = Path(xdg) / "golias";
            std::filesystem::create_directories(dir);
            return dir;
        }

        if (const char* home = std::getenv("HOME")) {
            Path dir = Path(home) / ".config" / "golias";
            std::filesystem::create_directories(dir);
            return dir;
        }
#endif

        return GetExecutablePath();
    }


    bool FileSystem::FileExists(const Path& path) const {
        return std::filesystem::exists(path);
    }

    std::vector<char> FileSystem::LoadFile(const Path& path) const {
        Path normalizedPath = path.lexically_normal();

        std::ifstream file(normalizedPath, std::ios::binary);

        if (!file.is_open()) {
            GOLIAS_LOG_ERROR("Failed to open file: %s", normalizedPath.string().c_str());
            return {};
        }


        file.seekg(0, std::ios::end);
        const std::streampos end = file.tellg();
        if (end < 0) {
            GOLIAS_LOG_ERROR("Failed to determine file size: %s", normalizedPath.string().c_str());
            return {};
        }

        const auto size = static_cast<std::size_t>(end);
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (size > 0 && !file.read(buffer.data(), static_cast<std::streamsize>(size))) {
            GOLIAS_LOG_ERROR("Failed to read file: %s", normalizedPath.string().c_str());
            return {};
        }

        GOLIAS_LOG_TRACE("Loaded file: %s (%zu bytes)", normalizedPath.string().c_str(), size);
        file.close();

        return buffer;
    }

    std::vector<char> FileSystem::LoadAssetFile(CString path) const {
        return LoadFile(GetAssetsFolder() / Path(path));
    }

    String FileSystem::LoadAssetFileText(CString path) const {
        std::vector<char> buffer = LoadAssetFile(path);

        return String(buffer.data(), buffer.size());
    }

    // TODO: We need to save file only to writable paths (e.g., user config directory).
    bool FileSystem::SaveFileText(const Path& path, CString contents) const {
        const Path normalizedPath = path.lexically_normal();

        std::error_code ec;
        std::filesystem::create_directories(normalizedPath.parent_path(), ec);

        std::ofstream file(normalizedPath, std::ios::binary);
        if (!file.is_open()) {
            GOLIAS_LOG_ERROR("Failed to open file for writing: %s", normalizedPath.string().c_str());
            return false;
        }

        file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        file.close();

        GOLIAS_LOG_TRACE("Saved file: %s (%zu bytes)", normalizedPath.string().c_str(), contents.size());
        return true;
    }

} // namespace golias
