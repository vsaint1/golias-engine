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

} // namespace golias
