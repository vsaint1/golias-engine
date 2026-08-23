#include "core/io/file_system.h"

namespace golias {

    const char* kAssetsFolderName = "Assets";

    Path FileSystem::GetExecutablePath() const {
        return std::filesystem::current_path();
    }

    Path FileSystem::GetAssetsFolder() const {
        return GetExecutablePath() / kAssetsFolderName;
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
        return LoadFile(GetAssetsFolder() / Path(std::string(path)));
    }

    String FileSystem::LoadAssetFileText(CString path) const {
        std::vector<char> buffer = LoadAssetFile(path);

        return String(buffer.data(), buffer.size());
    }
} // namespace golias
