#pragma once
#include "file_system.h"

/*!
 *  @brief MiniAudio VFS structure
 *
 * @version 0.0.5
 */
struct MiniAudio_VFS {
    ma_vfs_callbacks base;
};


ma_result ember_init_ma_vfs(MiniAudio_VFS* vfs);