#pragma once

#include "stdafx.h"

#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <string>
#include <memory>

class FileAccess;

/*!
 * @brief Custom Assimp IOStream implementation using SDL file access
 * @details Wraps FileAccess to provide Assimp with file reading capabilities
 */
class SDLIOStream : public Assimp::IOStream {
public:
    SDLIOStream(const std::string& path, const std::string& mode);
    ~SDLIOStream() override;

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
    size_t Tell() const override;
    size_t FileSize() const override;
    void Flush() override;

    std::unique_ptr<FileAccess> m_file;
    std::string m_path;
    size_t m_position = 0;
};

/*!
 * @brief Custom Assimp IOSystem implementation for asset loading
 * @details Allows Assimp to load models with external dependencies (like OBJ+MTL)
 *          through the engine's FileAccess system, supporting both desktop and Android
 */
class SDLIOSystem : public Assimp::IOSystem {
public:
    explicit SDLIOSystem(const std::string& base_path);
    ~SDLIOSystem() override;

    bool Exists(const char* pFile) const override;
    char getOsSeparator() const override;
    Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override;
    void Close(Assimp::IOStream* pFile) override;

private:
    std::string m_base_path;
};