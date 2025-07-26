#pragma once
#include  "engine_structs.h"


struct TextToken {
    std::string text;
    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    bool bold = false;
    bool italic = false;
    bool strike = false;

    static std::vector<TextToken> parse_bbcode(const std::string& text, const  glm::vec4& default_color);

};
