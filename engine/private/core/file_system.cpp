#include "core/file_system.h"


std::string LoadAssetsFile(const std::string& file_path) {

    const auto buffer = LoadFileIntoMemory(file_path);

    return std::string(buffer.begin(), buffer.end());
  
}


std::vector<char> LoadFileIntoMemory(const std::string& file_path){

    auto path = ASSETS_PATH + file_path;

    SDL_IOStream* file_rw = SDL_IOFromFile(path.c_str(), "rb");
    if (!file_rw) {
        LOG_ERROR("Failed to open file %s, ERROR: %s", path.c_str(), SDL_GetError());
        return {};

    }

    Sint64 size = SDL_GetIOSize(file_rw);
    if (size <= 0) {
        LOG_ERROR("Failed to get file size %s, ERROR: %s", path.c_str(), SDL_GetError());
        SDL_CloseIO(file_rw);
        return {};

    }

    std::vector<char> buffer(size);
    if (SDL_ReadIO(file_rw, buffer.data(), size) != size) {
        LOG_ERROR("Failed to read file %s, ERROR: %s", path.c_str(), SDL_GetError());
        SDL_CloseIO(file_rw);
        return {};
    }

    SDL_CloseIO(file_rw);

    return buffer;
}