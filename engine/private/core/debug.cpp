#include "core/debug.h"

namespace golias {

    void Debug::Log(const std::string& message) {
        spdlog::info("{}", message);
    }

    void Debug::LogWarning(const std::string& message) {
        spdlog::warn("{}", message);
    }

    void Debug::LogError(const std::string& message) {
        spdlog::error("{}", message);
    }

} // namespace golias
