#include "core/application.h"


namespace golias {
    bool Application::ShouldClose() const {
        return !is_running;
    }

    void Application::Close() {
        is_running = false;
    }
} // namespace golias
