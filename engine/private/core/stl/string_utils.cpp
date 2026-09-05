#include "core/stl/string_utils.h"


namespace golias {

    String String_Format(const char* format, ...) {
        va_list args;
        va_start(args, format);

        va_list copy;
        va_copy(copy, args);

        const int length = vsnprintf(nullptr, 0, format, copy);
        va_end(copy);

        String result;
        if (length > 0) {
            result.resize(static_cast<size_t>(length));
            vsnprintf(result.data(), static_cast<size_t>(length) + 1, format, args);
        }

        va_end(args);
        return result;
    }

    String String_FormatNumber(uint64_t value) {
        const String str = std::to_string(value);
        String result;
        result.reserve(str.size() + str.size() / 3);

        const size_t digits = str.size();
        for (size_t i = 0; i < digits; ++i) {
            if (i > 0 && (digits - i) % 3 == 0) {
                result.push_back(',');
            }
            result.push_back(str[i]);
        }

        return result;
    }

    String String_FormatBytes(uint64_t bytes) {
        const double gb = std::round(static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
        if (gb >= 1.0) {
            return String_Format("%.2f GB", gb);
        }

        const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
        if (mb >= 1.0) {
            return String_Format("%.1f MB", mb);
        }

        return String_Format("%llu B", static_cast<unsigned long long>(bytes));
    }

} // namespace golias
