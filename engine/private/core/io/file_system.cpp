#include "core/io/file_system.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <spdlog/spdlog.h>

#undef CreateDirectory

namespace golias {


    std::string FileSystem::GetExecutablePath() const {
        const char* path = SDL_GetBasePath();

        return path;
    }

    std::string FileSystem::GetAssetFile(const std::string_view pFileName) const {
        return (std::filesystem::path(GetAssetsPath()) / pFileName).string();
    }

    std::string FileSystem::GetAssetsPath() const {
        const char* base = SDL_GetBasePath();
        if (!base) {
            return "";
        }

        std::filesystem::path path(base);
        path /= "res";

        auto result = path.string();

        if (!result.empty() && result.back() != '/' && result.back() != '\\') {
            result.push_back(std::filesystem::path::preferred_separator);
        }

        return result;
    }

    bool FileSystem::FileExists(const std::string& path) const {
        return std::filesystem::exists(path);
    }

    bool FileSystem::CreateDirectory(const std::string& path) const {
        if (SDL_CreateDirectory(path.c_str())) {
            return true;
        }

        return false;
    }

    std::vector<char> FileSystem::LoadFile(const std::string_view pPath) const {
        SDL_IOStream* file = SDL_IOFromFile(pPath.data(), "rb");

        if (!file) {
            spdlog::error("FileSystem::LoadFile Failed to open file: {}", pPath);
            return {};
        }

        Sint64 size = SDL_GetIOSize(file);

        if (size <= 0) {
            spdlog::error("FileSystem::LoadFile Failed to get file size: {}", pPath);
            SDL_CloseIO(file);
            return {};
        }

        std::vector<char> buffer(static_cast<size_t>(size));

        Sint64 read_bytes = SDL_ReadIO(file, buffer.data(), size);

        if (read_bytes != size) {
            spdlog::error("FileSystem::LoadFile Failed to read file: {} (Read {} bytes, Expected {} bytes)", pPath, read_bytes, size);
            SDL_CloseIO(file);
            return {};
        }

        SDL_CloseIO(file);


        return buffer;
    }

    std::vector<char> FileSystem::LoadAssetFile(const std::string_view pFileName) const {
        return LoadFile(GetAssetsPath() + std::string(pFileName));
    }

    std::string FileSystem::LoadFileText(const std::string_view pPath) const {
        auto buffer = LoadFile(pPath);
        if (buffer.empty()) {
            return "";
        }

        return std::string(buffer.data(), buffer.size());
    }

    std::string FileSystem::LoadAssetFileText(const std::string_view pPath) const {
        auto buffer = LoadFile(GetAssetsPath() + std::string(pPath));
        if (buffer.empty()) {
            return "";
        }

        return std::string(buffer.data(), buffer.size());
    }

} // namespace golias
