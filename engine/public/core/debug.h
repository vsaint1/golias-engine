#pragma once

#include <string>
#include <spdlog/spdlog.h>

namespace golias {

    /// @brief Unity-style Debug class for logging and diagnostics.
    class Debug {
    public:
        /// Log an informational message
        static void Log(const std::string& message);

        /// Log a warning message
        static void LogWarning(const std::string& message);

        /// Log an error message
        static void LogError(const std::string& message);

        /// Log a formatted message (uses fmt syntax)
        template <typename... Args>
        static void LogFormat(fmt::format_string<Args...> fmt, Args&&... args) {
            spdlog::info(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void LogWarningFormat(fmt::format_string<Args...> fmt, Args&&... args) {
            spdlog::warn(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void LogErrorFormat(fmt::format_string<Args...> fmt, Args&&... args) {
            spdlog::error(fmt, std::forward<Args>(args)...);
        }
    };

} // namespace golias
