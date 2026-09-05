#pragma once
#include "stdafx.h"

namespace golias {

    /// @brief  printf-style formatting into a String.
    String String_Format(const char* format, ...);

    /// @brief  Formats an integer with thousands separators (e.g. 12480 -> "12,480").
    String String_FormatNumber(uint64_t value);

    /// @brief  Formats a byte count into a human readable string (e.g. "8.2 GB").
    String String_FormatBytes(uint64_t bytes);

} // namespace golias