#include "core/platform/memory.h"

#include <psapi.h>

namespace golias {


    MemoryStats GetMemoryStats() {
        MemoryStats stats = {};

        MEMORYSTATUSEX memoryStatus = {};
        memoryStatus.dwLength       = sizeof(memoryStatus);
        if (GlobalMemoryStatusEx(&memoryStatus)) {
            stats.TotalRamBytes = memoryStatus.ullTotalPhys;
            stats.UsedRamBytes  = memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys;
        }

        PROCESS_MEMORY_COUNTERS memoryCounters = {};
        if (GetProcessMemoryInfo(GetCurrentProcess(), &memoryCounters, sizeof(memoryCounters))) {
            stats.ProcessRamBytes = static_cast<uint64_t>(memoryCounters.WorkingSetSize);
        }

        if (GLAD_GL_NVX_gpu_memory_info || GLAD_GL_ATI_meminfo) {
            GLint totalKb     = 0;
            GLint availableKb = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableKb);

            if (totalKb > 0) {
                stats.TotalVramBytes = static_cast<uint64_t>(totalKb) * 1024ull;
                stats.VramAvailable  = true;
                stats.UsedVramBytes  = static_cast<uint64_t>(std::max(totalKb - availableKb, 0)) * 1024ull;
            }
        }

        return stats;
    }

} // namespace golias
