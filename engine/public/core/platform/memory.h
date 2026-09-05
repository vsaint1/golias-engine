#pragma once
#include "stdafx.h"

namespace golias {


    struct MemoryStats {
        uint64_t TotalRamBytes   = 0;
        uint64_t UsedRamBytes    = 0;
        uint64_t ProcessRamBytes = 0;
        uint64_t TotalVramBytes  = 0;
        uint64_t UsedVramBytes   = 0;
        bool VramAvailable       = false;
    };


    /// @brief Fetch memory statistics.
    MemoryStats GetMemoryStats();


} // namespace golias
