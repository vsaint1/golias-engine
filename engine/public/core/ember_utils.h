#pragma once

#include "stdafx.h"


std::vector<uint32_t> utf8_to_utf32(const std::string& utf8);

std::string utf32_to_utf8(uint32_t cp);

bool is_character_emoji(uint32_t cp);
