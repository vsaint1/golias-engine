#include "core/ember_utils.h"


std::vector<uint32_t> utf8_to_utf32(const std::string& utf8) {
    std::vector<uint32_t> result;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(utf8.c_str());
    size_t i                   = 0;
    while (i < utf8.length()) {
        uint32_t cp = 0;
        int n       = 1;
        if (bytes[i] <= 0x7F) {
            cp = bytes[i];
        } else if ((bytes[i] & 0xE0) == 0xC0) {
            n  = 2;
            cp = (bytes[i] & 0x1F) << 6;
            cp |= (bytes[i + 1] & 0x3F);
        } else if ((bytes[i] & 0xF0) == 0xE0) {
            n  = 3;
            cp = (bytes[i] & 0x0F) << 12;
            cp |= (bytes[i + 1] & 0x3F) << 6;
            cp |= (bytes[i + 2] & 0x3F);
        } else if ((bytes[i] & 0xF8) == 0xF0) {
            n  = 4;
            cp = (bytes[i] & 0x07) << 18;
            cp |= (bytes[i + 1] & 0x3F) << 12;
            cp |= (bytes[i + 2] & 0x3F) << 6;
            cp |= (bytes[i + 3] & 0x3F);
        }
        result.push_back(cp);
        i += n;
    }
    return result;
}

std::string utf32_to_utf8(uint32_t cp) {
    std::string r;
    if (cp <= 0x7F) {
        r += (char) cp;
    } else if (cp <= 0x7FF) {
        r += (char) (0xC0 | (cp >> 6));
        r += (char) (0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        r += (char) (0xE0 | (cp >> 12));
        r += (char) (0x80 | ((cp >> 6) & 0x3F));
        r += (char) (0x80 | (cp & 0x3F));
    } else {
        r += (char) (0xF0 | (cp >> 18));
        r += (char) (0x80 | ((cp >> 12) & 0x3F));
        r += (char) (0x80 | ((cp >> 6) & 0x3F));
        r += (char) (0x80 | (cp & 0x3F));
    }
    return r;
}

bool is_character_emoji(uint32_t cp) {
    return (cp >= 0x1F600 && cp <= 0x1F64F) || (cp >= 0x1F300 && cp <= 0x1F5FF) || (cp >= 0x1F680 && cp <= 0x1F6FF)
        || (cp >= 0x1F1E0 && cp <= 0x1F1FF) || (cp >= 0x2600 && cp <= 0x26FF) || (cp >= 0x2700 && cp <= 0x27BF)
        || (cp >= 0x1F900 && cp <= 0x1F9FF) || (cp >= 0x1FA70 && cp <= 0x1FAFF);
}
