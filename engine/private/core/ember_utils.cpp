#include "core/ember_utils.h"

std::string text_format(const char* fmt, ...) {
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
int get_memory_usage() {
    return 0;
}

int get_available_memory() {
    return SDL_GetSystemRAM();
}

int get_cpu_cores_num() {
    return SDL_GetNumLogicalCPUCores();
}


const char* file_extension_to_mime_type(const char* ext) {
    static const std::unordered_map<std::string, const char*> mime_types = {
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"txt", "text/plain"},
        {"csv", "text/csv"},
        {"xml", "text/xml"},
        {"pdf", "application/pdf"},
        {"doc", "application/msword"},
        {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {"ppt", "application/vnd.ms-powerpoint"},
        {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {"xls", "application/vnd.ms-excel"},
        {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"mp4", "video/mp4"},
        {"avi", "video/x-msvideo"},
        {"mov", "video/quicktime"},
        {"flv", "video/x-flv"},
        {"webm", "video/webm"},
        {"zip", "application/zip"},
        {"rar", "application/x-rar-compressed"},
        {"7z", "application/x-7z-compressed"},
        {"tar", "application/x-tar"},
        {"gz", "application/gzip"},
        {"mpg", "video/mpeg"},
        {"mpeg", "video/mpeg"},
        {"ogg", "application/ogg"},
        {"ogv", "video/ogg"},
        {"oga", "audio/ogg"},
        {"otf", "font/otf"},
        {"ttf", "font/ttf"},
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},
        {"eot", "application/vnd.ms-fontobject"},
        {"sfnt", "font/sfnt"},
        {"bin", "application/octet-stream"},
        {"exe", "application/octet-stream"},
        {"dll", "application/octet-stream"},
        {"psd", "image/vnd.adobe.photoshop"},
        {"ai", "application/postscript"},
        {"eps", "application/postscript"},
        {"ps", "application/postscript"},
        {"m4a", "audio/m4a"},
        {"m4v", "video/x-m4v"},
        {"bmp", "image/bmp"},
        {"tiff", "image/tiff"},
        {"tif", "image/tiff"},
        {"mkv", "video/x-matroska"},
        {"mpa", "video/mpeg"},
        {"mpe", "video/mpeg"},
        {"mid", "audio/midi"},
        {"midi", "audio/midi"},
        {"3gp", "video/3gpp"},
        {"3g2", "video/3gpp2"},
        {"aif", "audio/aiff"},
        {"aiff", "audio/aiff"},
        {"aac", "audio/aac"},
        {"au", "audio/basic"},
        {"wasm", "application/wasm"},
        {"xhtml", "application/xhtml+xml"},
        {"qt", "video/quicktime"}};

    if (const auto it = mime_types.find(std::string(ext)); it != mime_types.end()) {
        return it->second;
    }

    return "application/octet-stream"; // default MIME type if not found
}
