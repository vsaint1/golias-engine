#pragma once
#include "helpers/logging.h"


/*!

   @brief Loads a given file from `res` folder
   - The file path is relative to `res` folder

   @version 0.0.1
   @param file_path the path to the file in `res` folder
   @return File content as `string`
*/
std::string load_assets_file(const std::string& file_path);

/*!

   @brief Loads a given file from `res` folder into memory
   - The file path is relative to `res` folder

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

/*!
 *  @brief Engine virtual file system
 *
 *
 * @version 0.0.5
 */
struct Ember_VFS {
    ma_vfs_callbacks base;
};


ma_result _ember_init_vfs(Ember_VFS* vfs);
