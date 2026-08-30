#include "core/wsi/glfw_window.h"

#include <glad.h>

#if true

    #if defined(GOLIAS_PLATFORM_WINDOWS)
        #define GLFW_EXPOSE_NATIVE_WIN32
    #elif defined(GOLIAS_PLATFORM_OSX)
        #define GLFW_EXPOSE_NATIVE_COCOA
    #endif

    #include <GLFW/glfw3native.h>
#endif


namespace {

    KeyAction translate_key_action(int action) {
        switch (action) {
        case GLFW_PRESS:
            return KeyAction::Press;
        case GLFW_RELEASE:
            return KeyAction::Release;
        case GLFW_REPEAT:
            return KeyAction::Repeat;
        default:
            return KeyAction::Release;
        }
    }

    KeyCode translate_key_code(int key) {
        switch (key) {

            // Letters

        case GLFW_KEY_A:
            return KeyCode::A;
        case GLFW_KEY_B:
            return KeyCode::B;
        case GLFW_KEY_C:
            return KeyCode::C;
        case GLFW_KEY_D:
            return KeyCode::D;
        case GLFW_KEY_E:
            return KeyCode::E;
        case GLFW_KEY_F:
            return KeyCode::F;
        case GLFW_KEY_G:
            return KeyCode::G;
        case GLFW_KEY_H:
            return KeyCode::H;
        case GLFW_KEY_I:
            return KeyCode::I;
        case GLFW_KEY_J:
            return KeyCode::J;
        case GLFW_KEY_K:
            return KeyCode::K;
        case GLFW_KEY_L:
            return KeyCode::L;
        case GLFW_KEY_M:
            return KeyCode::M;
        case GLFW_KEY_N:
            return KeyCode::N;
        case GLFW_KEY_O:
            return KeyCode::O;
        case GLFW_KEY_P:
            return KeyCode::P;
        case GLFW_KEY_Q:
            return KeyCode::Q;
        case GLFW_KEY_R:
            return KeyCode::R;
        case GLFW_KEY_S:
            return KeyCode::S;
        case GLFW_KEY_T:
            return KeyCode::T;
        case GLFW_KEY_U:
            return KeyCode::U;
        case GLFW_KEY_V:
            return KeyCode::V;
        case GLFW_KEY_W:
            return KeyCode::W;
        case GLFW_KEY_X:
            return KeyCode::X;
        case GLFW_KEY_Y:
            return KeyCode::Y;
        case GLFW_KEY_Z:
            return KeyCode::Z;

            // Number row

        case GLFW_KEY_0:
            return KeyCode::Num0;
        case GLFW_KEY_1:
            return KeyCode::Num1;
        case GLFW_KEY_2:
            return KeyCode::Num2;
        case GLFW_KEY_3:
            return KeyCode::Num3;
        case GLFW_KEY_4:
            return KeyCode::Num4;
        case GLFW_KEY_5:
            return KeyCode::Num5;
        case GLFW_KEY_6:
            return KeyCode::Num6;
        case GLFW_KEY_7:
            return KeyCode::Num7;
        case GLFW_KEY_8:
            return KeyCode::Num8;
        case GLFW_KEY_9:
            return KeyCode::Num9;

            // Function keys

        case GLFW_KEY_F1:
            return KeyCode::F1;
        case GLFW_KEY_F2:
            return KeyCode::F2;
        case GLFW_KEY_F3:
            return KeyCode::F3;
        case GLFW_KEY_F4:
            return KeyCode::F4;
        case GLFW_KEY_F5:
            return KeyCode::F5;
        case GLFW_KEY_F6:
            return KeyCode::F6;
        case GLFW_KEY_F7:
            return KeyCode::F7;
        case GLFW_KEY_F8:
            return KeyCode::F8;
        case GLFW_KEY_F9:
            return KeyCode::F9;
        case GLFW_KEY_F10:
            return KeyCode::F10;
        case GLFW_KEY_F11:
            return KeyCode::F11;
        case GLFW_KEY_F12:
            return KeyCode::F12;
        case GLFW_KEY_F13:
            return KeyCode::F13;
        case GLFW_KEY_F14:
            return KeyCode::F14;
        case GLFW_KEY_F15:
            return KeyCode::F15;
        case GLFW_KEY_F16:
            return KeyCode::F16;
        case GLFW_KEY_F17:
            return KeyCode::F17;
        case GLFW_KEY_F18:
            return KeyCode::F18;
        case GLFW_KEY_F19:
            return KeyCode::F19;
        case GLFW_KEY_F20:
            return KeyCode::F20;
        case GLFW_KEY_F21:
            return KeyCode::F21;
        case GLFW_KEY_F22:
            return KeyCode::F22;
        case GLFW_KEY_F23:
            return KeyCode::F23;
        case GLFW_KEY_F24:
            return KeyCode::F24;
        case GLFW_KEY_F25:
            return KeyCode::F25;

            // Modifiers

        case GLFW_KEY_LEFT_SHIFT:
            return KeyCode::LeftShift;

        case GLFW_KEY_RIGHT_SHIFT:
            return KeyCode::RightShift;

        case GLFW_KEY_LEFT_CONTROL:
            return KeyCode::LeftControl;

        case GLFW_KEY_RIGHT_CONTROL:
            return KeyCode::RightControl;

        case GLFW_KEY_LEFT_ALT:
            return KeyCode::LeftAlt;

        case GLFW_KEY_RIGHT_ALT:
            return KeyCode::RightAlt;

        case GLFW_KEY_LEFT_SUPER:
            return KeyCode::LeftSuper;

        case GLFW_KEY_RIGHT_SUPER:
            return KeyCode::RightSuper;

            // Navigation / control

        case GLFW_KEY_ESCAPE:
            return KeyCode::Escape;

        case GLFW_KEY_ENTER:
            return KeyCode::Enter;

        case GLFW_KEY_TAB:
            return KeyCode::Tab;

        case GLFW_KEY_BACKSPACE:
            return KeyCode::Backspace;

        case GLFW_KEY_INSERT:
            return KeyCode::Insert;

        case GLFW_KEY_DELETE:
            return KeyCode::Delete;

        case GLFW_KEY_HOME:
            return KeyCode::Home;

        case GLFW_KEY_END:
            return KeyCode::End;

        case GLFW_KEY_PAGE_UP:
            return KeyCode::PageUp;

        case GLFW_KEY_PAGE_DOWN:
            return KeyCode::PageDown;

            // Arrows

        case GLFW_KEY_LEFT:
            return KeyCode::Left;

        case GLFW_KEY_RIGHT:
            return KeyCode::Right;

        case GLFW_KEY_UP:
            return KeyCode::Up;

        case GLFW_KEY_DOWN:
            return KeyCode::Down;

            // Lock / system

        case GLFW_KEY_CAPS_LOCK:
            return KeyCode::CapsLock;

        case GLFW_KEY_NUM_LOCK:
            return KeyCode::NumLock;

        case GLFW_KEY_SCROLL_LOCK:
            return KeyCode::ScrollLock;

        case GLFW_KEY_PRINT_SCREEN:
            return KeyCode::PrintScreen;

        case GLFW_KEY_PAUSE:
            return KeyCode::Pause;

        case GLFW_KEY_MENU:
            return KeyCode::Menu;

            // Punctuation

        case GLFW_KEY_SPACE:
            return KeyCode::Space;

        case GLFW_KEY_APOSTROPHE:
            return KeyCode::Apostrophe;

        case GLFW_KEY_COMMA:
            return KeyCode::Comma;

        case GLFW_KEY_MINUS:
            return KeyCode::Minus;

        case GLFW_KEY_PERIOD:
            return KeyCode::Period;

        case GLFW_KEY_SLASH:
            return KeyCode::Slash;

        case GLFW_KEY_SEMICOLON:
            return KeyCode::Semicolon;

        case GLFW_KEY_EQUAL:
            return KeyCode::Equal;

        case GLFW_KEY_LEFT_BRACKET:
            return KeyCode::LeftBracket;

        case GLFW_KEY_BACKSLASH:
            return KeyCode::Backslash;

        case GLFW_KEY_RIGHT_BRACKET:
            return KeyCode::RightBracket;

        case GLFW_KEY_GRAVE_ACCENT:
            return KeyCode::GraveAccent;

            // Keypad

        case GLFW_KEY_KP_0:
            return KeyCode::KP0;
        case GLFW_KEY_KP_1:
            return KeyCode::KP1;
        case GLFW_KEY_KP_2:
            return KeyCode::KP2;
        case GLFW_KEY_KP_3:
            return KeyCode::KP3;
        case GLFW_KEY_KP_4:
            return KeyCode::KP4;
        case GLFW_KEY_KP_5:
            return KeyCode::KP5;
        case GLFW_KEY_KP_6:
            return KeyCode::KP6;
        case GLFW_KEY_KP_7:
            return KeyCode::KP7;
        case GLFW_KEY_KP_8:
            return KeyCode::KP8;
        case GLFW_KEY_KP_9:
            return KeyCode::KP9;

        case GLFW_KEY_KP_DECIMAL:
            return KeyCode::KPDecimal;

        case GLFW_KEY_KP_DIVIDE:
            return KeyCode::KPDivide;

        case GLFW_KEY_KP_MULTIPLY:
            return KeyCode::KPMultiply;

        case GLFW_KEY_KP_SUBTRACT:
            return KeyCode::KPSubtract;

        case GLFW_KEY_KP_ADD:
            return KeyCode::KPAdd;

        case GLFW_KEY_KP_ENTER:
            return KeyCode::KPEnter;

        case GLFW_KEY_KP_EQUAL:
            return KeyCode::KPEqual;

            // International

        case GLFW_KEY_WORLD_1:
            return KeyCode::World1;

        case GLFW_KEY_WORLD_2:
            return KeyCode::World2;

        default:
            return KeyCode::Unknown;
        }
    }

    bool translate_mouse_action(int action) {
        return action == GLFW_PRESS;
    }

    MouseButton translate_mouse_button(int button) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:
            return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5:
            return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6:
            return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7:
            return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8:
            return MouseButton::Button8;
        default:
            return MouseButton::Count;
        }
    }

    int translate_input_mode(golias::InputMode mode) {
        switch (mode) {
        case golias::InputMode::Cursor:
            return GLFW_CURSOR_NORMAL;
        case golias::InputMode::Disabled:
            return GLFW_CURSOR_DISABLED;
        default:
            return GLFW_CURSOR_NORMAL;
        }
    }
} // namespace


namespace golias {

    void GlfwWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        win->mWidth     = width;
        win->mHeight    = height;

        if (win->OnResize) {
            win->OnResize(width, height);
        }
    }

    void GlfwWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        if (!win || !win->OnMouseButton) {
            return;
        }

        win->OnMouseButton(translate_mouse_button(button), translate_mouse_action(action), mods);
    }

    void GlfwWindow::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        if (!win || !win->OnScroll) {
            return;
        }

        win->OnScroll(xoffset, yoffset);
    }

    void GlfwWindow::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
        GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        if (!win || !win->OnCursorPos) {
            return;
        }

        win->OnCursorPos(xpos, ypos);
    }

    void GlfwWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        if (!win || !win->OnKey) {
            return;
        }

        win->OnKey(translate_key_code(key), translate_key_action(action), mods);
    }

    GlfwWindow::GlfwWindow(int width, int height, CString title) : Window(width, height, title) {

#if defined(GOLIAS_PLATFORM_OSX)
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);
#endif

#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    #if defined(GOLIAS_PLATFORM_OSX)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif


#endif

        if (!glfwInit()) {
            GOLIAS_LOG_CRITICAL("Failed to initialize GLFW.");
            glfwTerminate();
            return;
        }

        mWindow = glfwCreateWindow(mWidth, mHeight, title.data(), nullptr, nullptr);
        if (!mWindow) {
            GOLIAS_LOG_CRITICAL("Failed to create window.");
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(static_cast<GLFWwindow*>(mWindow));

#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return;
        }

#elif defined(GOLIAS_PLATFORM_ANDROID) || defined(GOLIAS_PLATFORM_IOS)

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return;
        }

#endif


        glfwSetWindowUserPointer(static_cast<GLFWwindow*>(mWindow), this);
        glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(mWindow), GlfwWindow::framebuffer_size_callback);
        glfwSetKeyCallback(static_cast<GLFWwindow*>(mWindow), GlfwWindow::key_callback);
        glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(mWindow), GlfwWindow::mouse_button_callback);
        glfwSetScrollCallback(static_cast<GLFWwindow*>(mWindow), GlfwWindow::scroll_callback);
        glfwSetCursorPosCallback(static_cast<GLFWwindow*>(mWindow), GlfwWindow::cursor_position_callback);

        glfwSetCursorPos(static_cast<GLFWwindow*>(mWindow), mWidth / 2.0, mHeight / 2.0);

        GOLIAS_LOG_INFO("Created window with size %dx%d", mWidth, mHeight);
    }

    GlfwWindow::~GlfwWindow() {
        if (mWindow) {
            glfwDestroyWindow(static_cast<GLFWwindow*>(mWindow));
            mWindow = nullptr;
        }

        glfwTerminate();
    }

    void GlfwWindow::SwapBuffers() {
        if (mWindow) {
            glfwSwapBuffers(static_cast<GLFWwindow*>(mWindow));
        }
    }

    void GlfwWindow::PollEvents() {
        glfwPollEvents();
    }

    bool GlfwWindow::ShouldClose() const {
        return mWindow && glfwWindowShouldClose(static_cast<GLFWwindow*>(mWindow));
    }

    void GlfwWindow::Close() {
        if (mWindow) {
            glfwSetWindowShouldClose(static_cast<GLFWwindow*>(mWindow), GLFW_TRUE);
        }
    }

    const String& GlfwWindow::GetTitle() const {
        return mTitle;
    }

    void GlfwWindow::SetTitle(CString title) {
        mTitle = title;
        if (mWindow) {
            glfwSetWindowTitle(static_cast<GLFWwindow*>(mWindow), mTitle.data());
        }
    }

    void GlfwWindow::GetDrawableSize(int* width, int* height) const {
        if (mWindow) {
            glfwGetFramebufferSize(static_cast<GLFWwindow*>(mWindow), width, height);
        } else {
            *width  = 0;
            *height = 0;
        }
    }

    void GlfwWindow::WaitForEvents() {
        glfwWaitEvents();
    }


    void* GlfwWindow::GetNativeHandle() const {
#if defined(GOLIAS_PLATFORM_OSX)
        if (!mWindow || glfwGetPlatform() != GLFW_PLATFORM_COCOA) {
            return nullptr;
        }
        return glfwGetCocoaWindow(static_cast<GLFWwindow*>(mWindow));

#elif defined(GOLIAS_PLATFORM_WINDOWS)
        HWND hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(mWindow));
        return hwnd;
#endif

        GOLIAS_ASSERT_MSG(false, "Unsupported platform for GetNativeHandle");
        return nullptr;
    }

    void* GlfwWindow::GetNativeViewHandle() const {

#if defined(GOLIAS_PLATFORM_OSX)
        if (!mWindow || glfwGetPlatform() != GLFW_PLATFORM_COCOA) {
            return nullptr;
        }

        return glfwGetCocoaView(static_cast<GLFWwindow*>(mWindow));
#endif
        GOLIAS_ASSERT_MSG(false, "Unsupported platform for GetNativeViewHandle");
        return nullptr;
    }

    void* GlfwWindow::GetHandle() const {
        return mWindow;
    }

    InputMode GlfwWindow::GetInputMode() const {
        return mInputMode;
    }

    void GlfwWindow::SetInputMode(InputMode mode) {
        mInputMode = mode;
        if (mWindow) {
            glfwSetInputMode(static_cast<GLFWwindow*>(mWindow), GLFW_CURSOR, translate_input_mode(mode));
        }
    }

} // namespace golias
