#include "core/application.h"

namespace golias {
    
    bool Application::ShouldClose() const {
        return !mIsRunning;
    }

    void Application::Close() {
        mIsRunning = false;
    }
}