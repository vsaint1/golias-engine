#pragma once
#include "helpers/logging.h"


/*!

   @brief Loads a given file from `assets` folder
   - The file path is relative to `assets` folder

   @version 0.0.1
   @param file_path the path to the file in `assets` folder
   @return File content as `string`
*/
std::string LoadAssetsFile(const std::string& file_path);

