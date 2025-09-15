#pragma once

#include  "core/systems/logging_sys.h"


struct Packet {
    uint8_t type;
    std::vector<uint8_t> data;
};