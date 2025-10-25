#pragma once
#include  "stdafx.h"

/*!

   @brief Loads a given file from `res` folder
   - The file path is relative to `res` folder

   @ingroup FileSystem
   @version 0.0.1
   @param file_path the path to the file in `res` folder
   @return File content as `string`
*/
std::string load_assets_file(const std::string& file_path);

/*!

   @brief Loads a given file from `res` folder into memory
   - The file path is relative to `res` folder
   @ingroup FileSystem

   @version 0.0.2
   @param file_path the path to the file in `res` folder
   @return File content as `vector<char>`
*/
std::vector<char> load_file_into_memory(const std::string& file_path);

/*!
 *  @brief Ember file (SDL_stream)
 *
 *
 *  @version 0.0.8
 */
struct Ember_File {
    SDL_IOStream* stream;
};



/**
 * @brief Flags to indicate access modes.
 *
 *
 * - READ       : Read-only access
 * - WRITE      : Write-only access
 * - READ_WRITE : Read + Write access (read first, then write)
 * - WRITE_READ : Write + Read access (write first, then read)
 * @ingroup FileSystem
 */
enum class ModeFlags {
    READ, ///< Read-only access
    WRITE, ///< Write-only access
    READ_WRITE, ///< Read then Write access
    WRITE_READ ///< Write then Read access
};

/**
 * @class FileAccess
 * @brief A RAII wrapper for SDL3 file operations supporting read/write access.
 *
 * - `res://` (assets) and `user://` (writable user data) paths.
 * @ingroup FileSystem
 */
class FileAccess {
public:
    /**
     * @brief Construct and optionally open a file.
     * @param file_path Path to the file (can be res:// or user://)
     * @param mode_flags Mode of access (default: READ)
     */
    explicit FileAccess(const std::string& file_path, ModeFlags mode_flags = ModeFlags::READ);

    /**
     * @brief Default constructor (empty, file not opened)
     */
    FileAccess() = default;

    /**
     * @brief Destructor automatically closes the file if it is open
     */
    ~FileAccess();

    /**
     * @brief Open a file for reading/writing.
     * @param file_path Path to the file (res:// or user://)
     * @param mode_flags Access mode
     *
     * @details  By default, get from `res://`
     * @return true if the file was successfully opened, false otherwise
     */
    bool open(const std::string& file_path, ModeFlags mode_flags = ModeFlags::READ);

    /**
     * @brief Check if the file is currently open.
     * @return true if open, false otherwise
     */
    [[nodiscard]] bool is_open() const;

    /**
     * @brief Read the entire file as a vector of bytes.
     * @return File contents as Array of Bytes.
     */
    std::vector<char> get_file_as_bytes();

    /**
     * @brief Check if a file exists at the given path.
     * @param file_path Path to the file
     * @return true if the file exists, false otherwise
     */
    static bool file_exists(const std::string& file_path);

    /**
     * @brief Read the entire file as a String.
     * @return File contents as String
     */
    std::string get_file_as_str();

    /**
     * @brief Get the absolute resolved path to the file.
     * @return Full path on the filesystem
     */
    [[nodiscard]] std::string get_absolute_path() const;

    /**
     * @brief Get the path relative to the base (removes res:// or user:// prefix)
     * @return Relative path string
     */
    [[nodiscard]] std::string get_path() const;

    /**
     * @brief Write a string to the file.
     * @param content String content to write
     * @return true if successful
     */
    bool store_string(const std::string& content = "");

    /**
     * @brief Write a byte array to the file.
     * @param content Vector of bytes to write
     * @return true if successful
     */
    bool store_bytes(const std::vector<char>& content);

    /**
     * @brief Move the file pointer to the given offset from the beginning.
     * @param length Offset in bytes
     */
    void seek(int length);

    /**
     * @brief Move the file pointer relative to the end of the file.
     * @param position Offset in bytes from the end
     */
    void seek_end(int position);

    /**
     * @brief Close the file if it is open.
     */
    void close();

    FileAccess(const FileAccess&) = delete;

    FileAccess& operator=(const FileAccess&) = delete;

    SDL_IOStream* get_handle() const {
        return _file;
    }

private:
    SDL_IOStream* _file = nullptr; ///< Internal SDL file handle
    std::string _file_path; ///< Full resolved file path

    bool resolve_path(const std::string& file_path, ModeFlags mode_flags);
};