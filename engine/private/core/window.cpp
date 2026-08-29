#include "core/window.h"

namespace golias {


    Window::Window(int width, int height, CString title) : mWidth(width), mHeight(height), mTitle(title) {
    }

    Window::~Window() {
    }

    int Window::GetWidth() const {
        return mWidth;
    }

    int Window::GetHeight() const {
        return mHeight;
    }

} // namespace golias
