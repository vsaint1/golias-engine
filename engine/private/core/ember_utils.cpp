#include "core/ember_utils.h"

std::string TextFormat(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int size = SDL_vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (size <= 0) {
        return "";
    }

    std::string buffer(size, '\0');

    va_start(args, fmt);
    SDL_vsnprintf(&buffer[0], size + 1, fmt, args);
    va_end(args);

    return buffer;
}

#if defined(_WIN32)
#include <windows.h>
int GetMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    DWORDLONG usedMemory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    return static_cast<int>(usedMemory) / (1024.0f * 1024.0f);
}
#else
int GetMemoryUsage() {
    return 0;
}
#endif
