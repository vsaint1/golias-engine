#include "core/renderer/renderer.h"


void Renderer::set_default_fonts(const std::string& text_font, const std::string& emoji_font) {
    _default_font_name = text_font;
    _emoji_font_name   = emoji_font;
}


std::string Renderer::vformat(const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    return std::string(buf);
}
