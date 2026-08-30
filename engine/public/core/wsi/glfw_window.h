#pragma once
#include "core/window.h"
#include <glfw/glfw3.h>

namespace golias {


    class GlfwWindow final : public Window {
    public:
        GlfwWindow(int width, int height, CString title = "Golias Engine");
        ~GlfwWindow() override;

        void PollEvents() override;
        bool ShouldClose() const override;
        void Close() override;

        const String& GetTitle() const override;
        void SetTitle(CString title) override;

        InputMode GetInputMode() const override;
        void SetInputMode(InputMode mode) override;

        void GetDrawableSize(int* width, int* height) const override;

        void* GetNativeHandle() const override;

        void* GetNativeViewHandle() const override;

        void* GetHandle() const override;

        void WaitForEvents() override;

        void SwapBuffers() override;

    private:
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    };
} // namespace golias
