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

// TODO: implement
int GetMemoryUsage() {
    return 0;
}
