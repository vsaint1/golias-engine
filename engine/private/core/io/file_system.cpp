#include "core/io/file_system.h"



std::string load_assets_file(const std::string& file_path) {

    spdlog::debug("Loading asset file: {}", file_path);
    const auto buffer = load_file_into_memory(file_path);

    return std::string(buffer.begin(), buffer.end());
}


std::vector<char> load_file_into_memory(const std::string& file_path) {

    auto path = ASSETS_PATH + file_path;

    SDL_IOStream* file_rw = SDL_IOFromFile(path.c_str(), "rb");
    if (!file_rw) {
        spdlog::error("Failed to open file {} , Error: {}", path.c_str(), SDL_GetError());
        return {};
    }

    Sint64 size = SDL_GetIOSize(file_rw);
    if (size <= 0) {
        spdlog::error("Failed to get file size {}, Error: {}", path.c_str(), SDL_GetError());
        SDL_CloseIO(file_rw);
        return {};
    }

    std::vector<char> buffer(size);
    if (SDL_ReadIO(file_rw, buffer.data(), size) != size) {
        spdlog::error("Failed to read file {}", path.c_str());
        SDL_CloseIO(file_rw);
        return {};
    }

    SDL_CloseIO(file_rw);

    return buffer;
}


const char* get_mode_str(ModeFlags mode_flags) {
    switch (mode_flags) {
    case ModeFlags::READ:
        return "rb";
    case ModeFlags::WRITE:
        return "wb";
    case ModeFlags::READ_WRITE:
        return "r+b";
    case ModeFlags::WRITE_READ:
        return "w+b";
    default:
        return "rb";
    }
}

bool FileAccess::resolve_path(const std::string& file_path, ModeFlags mode_flags) {

    if (file_path.rfind("res://", 0) == 0) {
        _file_path = ASSETS_PATH + file_path.substr(6);
    }
    else if (file_path.rfind("user://", 0) == 0) {
        char* prefPath = SDL_GetPrefPath(ENGINE_DEFAULT_FOLDER_NAME, ENGINE_PACKAGE_NAME);
        if (!prefPath) {
            spdlog::error("Failed to get pref path: {}", SDL_GetError());
            return false;
        }

        _file_path = std::string(prefPath) + file_path.substr(7);
        SDL_free(prefPath);

        if (mode_flags == ModeFlags::WRITE || mode_flags == ModeFlags::READ_WRITE || mode_flags == ModeFlags::WRITE_READ) {
            std::size_t slashPos = _file_path.find_last_of("/\\");
            if (slashPos != std::string::npos) {
                const std::string dir = _file_path.substr(0, slashPos);
                if (!SDL_CreateDirectory(dir.c_str())) {
                    spdlog::error("Failed to create directory {}, Error: {}", dir.c_str(),SDL_GetError());
                    return false;
                }
            }
        }
    }
    else {
        _file_path =  file_path;
    }

    return true;
}

bool FileAccess::open(const std::string& file_path, ModeFlags mode_flags) {
    close();

    if (!resolve_path(file_path, mode_flags)) {
        return false;
    }

    _file = SDL_IOFromFile(_file_path.c_str(), get_mode_str(mode_flags));
    if (!_file) {
        spdlog::error("Failed to open file {}", _file_path.c_str());
        return false;
    }

    return true;
}

FileAccess::FileAccess(const std::string& file_path, ModeFlags mode_flags) {
    open(file_path, mode_flags);
}

FileAccess::~FileAccess() {
    close();
}

bool FileAccess::is_open() const {
    return _file != nullptr;
}

std::vector<char> FileAccess::get_file_as_bytes() {
    std::vector<char> buffer;
    if (!_file) return buffer;

    Sint64 size = SDL_GetIOSize(_file);
    if (size <= 0) return buffer;

    buffer.resize(size);
    Sint64 read = SDL_ReadIO(_file, buffer.data(), size);
    if (read != size) {
        spdlog::error("Failed to read file %s", _file_path.c_str());
    }

    SDL_SeekIO(_file, 0, SDL_IO_SEEK_SET); // rewind
    return buffer;
}

std::string FileAccess::get_file_as_str() {
    auto bytes = get_file_as_bytes();
    return {bytes.begin(), bytes.end()};
}

bool FileAccess::file_exists(const std::string& file_path) {
    std::string path;

    if (file_path.rfind("res://", 0) == 0) {
        path = ASSETS_PATH + file_path.substr(6);
    } else if (file_path.rfind("user://", 0) == 0) {
        char* prefPath = SDL_GetPrefPath("Golias", "com.golias.engine.app");
        if (!prefPath) return false;
        path = std::string(prefPath) + file_path.substr(7);
        SDL_free(prefPath);
    } else {
        path = ASSETS_PATH + file_path;
    }

    SDL_IOStream* test = SDL_IOFromFile(path.c_str(), "rb");
    if (test) {
        SDL_CloseIO(test);
        return true;
    }
    return false;
}

void FileAccess::seek(int length) {
    if (_file) SDL_SeekIO(_file, length, SDL_IO_SEEK_SET);
}

void FileAccess::seek_end(int position) {
    if (_file) SDL_SeekIO(_file, position, SDL_IO_SEEK_END);
}

std::string FileAccess::get_absolute_path() const {
    return _file_path;
}

std::string FileAccess::get_path() const {
    std::string path = _file_path;
    std::size_t pos = path.find("//");
    if (pos != std::string::npos && pos + 2 < path.size()) {
        path = path.substr(pos + 2);
    }
    return path;
}

bool FileAccess::store_string(const std::string& content) {
    if (!_file) {
        spdlog::error("File not open for writing: %s", _file_path.c_str());
        return false;
    }

    Sint64 written = SDL_WriteIO(_file, content.data(), content.size());
    if (written != static_cast<Sint64>(content.size())) {
        spdlog::error("Failed to write string to file %s", _file_path.c_str());
        return false;
    }

    return true;
}

bool FileAccess::store_bytes(const std::vector<char>& content) {
    if (!_file) {
        spdlog::error("File not open for writing: %s", _file_path.c_str());
        return false;
    }
    if (content.empty()) return true;

    Sint64 written = SDL_WriteIO(_file, content.data(), content.size());
    if (written != static_cast<Sint64>(content.size())) {
        spdlog::error("Failed to write bytes to file %s", _file_path.c_str());
        return false;
    }

    return true;
}

void FileAccess::close() {
    if (_file) {
        SDL_CloseIO(_file);
        _file = nullptr;
    }
}