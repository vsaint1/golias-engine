#include "core/io/assimp_io.h"
#include "core/io/file_system.h"


SDLIOStream::SDLIOStream(const std::string& path, const std::string& mode) 
    : m_path(path) {
    
    ModeFlags flags = ModeFlags::READ;
    if (mode.find('w') != std::string::npos) {
        flags = ModeFlags::WRITE;
    }
    
    m_file = std::make_unique<FileAccess>(path, flags);
}

SDLIOStream::~SDLIOStream() {
    // FileAccess destructor handles cleanup
}

size_t SDLIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount) {
    if (!m_file || !m_file->is_open()) {
        return 0;
    }
    
    const auto& bytes = m_file->get_file_as_bytes();
    size_t bytesToRead = pSize * pCount;
    size_t available = bytes.size() - m_position;
    size_t toRead = std::min(bytesToRead, available);
    
    if (toRead > 0) {
        SDL_memcpy(pvBuffer, bytes.data() + m_position, toRead);
        m_position += toRead;
    }
    
    return toRead / pSize;
}

size_t SDLIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount) {
    // Write not implemented for assets
    return 0;
}

aiReturn SDLIOStream::Seek(size_t pOffset, aiOrigin pOrigin) {
    if (!m_file || !m_file->is_open()) {
        return aiReturn_FAILURE;
    }
    
    size_t fileSize = m_file->get_file_as_bytes().size();
    
    switch (pOrigin) {
        case aiOrigin_SET:
            m_position = pOffset;
            break;
        case aiOrigin_CUR:
            m_position += pOffset;
            break;
        case aiOrigin_END:
            m_position = fileSize + pOffset;
            break;
        default:
            return aiReturn_FAILURE;
    }
    
    return aiReturn_SUCCESS;
}

size_t SDLIOStream::Tell() const {
    return m_position;
}

size_t SDLIOStream::FileSize() const {
    if (!m_file || !m_file->is_open()) {
        return 0;
    }
    return m_file->get_file_as_bytes().size();
}

void SDLIOStream::Flush() {
    // Nothing to flush for read-only files
}

SDLIOSystem::SDLIOSystem(const std::string& base_path) 
    : m_base_path(base_path) {
    // Ensure base path ends with separator
    if (!m_base_path.empty() && m_base_path.back() != '/' && m_base_path.back() != '\\') {
        m_base_path += '/';
    }
}

SDLIOSystem::~SDLIOSystem() {
    // Cleanup handled by unique_ptrs
}

bool SDLIOSystem::Exists(const char* pFile) const {
    std::string full_path = m_base_path + pFile;
    FileAccess file(full_path, ModeFlags::READ);
    return file.is_open();
}

char SDLIOSystem::getOsSeparator() const {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

Assimp::IOStream* SDLIOSystem::Open(const char* pFile, const char* pMode) {
    std::string full_path = m_base_path + pFile;
    
    auto stream = new SDLIOStream(full_path, pMode);
    if (!stream->m_file || !stream->m_file->is_open()) {
        delete stream;
        return nullptr;
    }
    
    return stream;
}

void SDLIOSystem::Close(Assimp::IOStream* pFile) {
    delete pFile;
}