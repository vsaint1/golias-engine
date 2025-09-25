#pragma once

#include "stdafx.h"


struct Tokens {
    std::string text;
    bool is_emoji;
};


class Font {
public:
    virtual ~Font() = default;
    virtual glm::vec2 get_size(const std::string& text) {
        return {0, 0};
    }
};