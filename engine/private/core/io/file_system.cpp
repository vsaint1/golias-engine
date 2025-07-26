#include "core/io/file_system.h"


std::string _load_assets_file(const std::string& file_path) {

    const auto buffer = _load_file_into_memory(file_path);

    return std::string(buffer.begin(), buffer.end());

}


std::vector<char> _load_file_into_memory(const std::string& file_path){

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

static ma_result sdl_vfs_onOpen(ma_vfs* pVFS, const char* pPath, ma_uint32 openMode, ma_vfs_file* pFile) {
    if (!pVFS || !pPath || !pFile) {
        return MA_INVALID_ARGS;
    }

    if (openMode != MA_OPEN_MODE_READ) {
        return MA_INVALID_ARGS;
    }

    SDL_IOStream* rw = SDL_IOFromFile(pPath, "rb");
    if (!rw) {
        return MA_DOES_NOT_EXIST;
    }

    Ember_File* file = (Ember_File*) SDL_malloc(sizeof(Ember_File));
    if (!file) {
        SDL_CloseIO(rw);
        return MA_OUT_OF_MEMORY;
    }

    file->stream = rw;
    *pFile   = file;
    return MA_SUCCESS;
}

static ma_result sdl_vfs_onRead(ma_vfs* pVFS, ma_vfs_file file, void* pBuffer, size_t size, size_t* pBytesRead) {
    if (!file || !pBuffer || !pBytesRead) {
        return MA_INVALID_ARGS;
    }

    Ember_File* sdlFile = (Ember_File*) file;
    size_t bytesRead  = SDL_ReadIO(sdlFile->stream, pBuffer, size);
    *pBytesRead       = bytesRead;

    return (bytesRead > 0) ? MA_SUCCESS : MA_AT_END;
}

static ma_result sdl_vfs_onSeek(ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin) {
    if (!file) {
        return MA_INVALID_ARGS;
    }

    Ember_File* sdlFile   = (Ember_File*) file;
    SDL_IOWhence whence = (origin == ma_seek_origin_start)   ? SDL_IO_SEEK_SET
                        : (origin == ma_seek_origin_current) ? SDL_IO_SEEK_CUR
                                                             : SDL_IO_SEEK_END;

    return (SDL_SeekIO(sdlFile->stream, offset, whence) < 0) ? MA_ERROR : MA_SUCCESS;
}

static ma_result sdl_vfs_onTell(ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor) {
    if (!file || !pCursor) {
        return MA_INVALID_ARGS;
    }

    Ember_File* sdlFile = (Ember_File*) file;
    ma_int64 pos      = SDL_TellIO(sdlFile->stream);
    if (pos < 0) {
        return MA_ERROR;
    }

    *pCursor = pos;
    return MA_SUCCESS;
}

static ma_result sdl_vfs_onClose(ma_vfs* pVFS, ma_vfs_file file) {
    if (!file) {
        return MA_INVALID_ARGS;
    }

    Ember_File* sdlFile = (Ember_File*) file;
    SDL_CloseIO(sdlFile->stream);
    SDL_free(sdlFile);

    return MA_SUCCESS;
}

ma_result _ember_init_vfs(Ember_VFS* vfs) {
    if (!vfs) {
        return MA_INVALID_ARGS;
    }

    vfs->base.onOpen  = sdl_vfs_onOpen;
    vfs->base.onRead  = sdl_vfs_onRead;
    vfs->base.onSeek  = sdl_vfs_onSeek;
    vfs->base.onTell  = sdl_vfs_onTell;
    vfs->base.onClose = sdl_vfs_onClose;
    vfs->base.onWrite = NULL;

    return MA_SUCCESS;
}
