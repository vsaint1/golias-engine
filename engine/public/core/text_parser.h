#pragma once
#include  "engine_structs.h"


struct TextToken {
    std::string text;
    Color color;
    bool bold = false;
    bool italic = false;
    bool strike = false;

    static std::vector<TextToken> parse_bbcode(const std::string& text, const Color& default_color);

};
