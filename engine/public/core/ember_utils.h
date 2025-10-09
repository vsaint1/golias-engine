#pragma once

#include "stdafx.h"


std::vector<uint32_t> utf8_to_utf32(const std::string& utf8);

std::string utf32_to_utf8(uint32_t cp);

bool is_character_emoji(uint32_t cp);

template <typename NumType>
inline NumType random_number(NumType min, NumType max) {
    static_assert(std::is_arithmetic_v<NumType>, "random_number requires an arithmetic type");
    static std::random_device rd;
    static std::mt19937       gen(rd());
    std::uniform_real_distribution<NumType> dis(min, max);
    return dis(gen);
}
