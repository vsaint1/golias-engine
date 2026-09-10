#include "editor/imgui_context.h"

#include "core/window.h"
#include <GLFW/glfw3.h>

#if defined(GOLIAS_WITH_EDITOR)
    #include <imgui.h>
    #include <imgui_impl_glfw.h>
    #include <imgui_impl_opengl3.h>
#endif

namespace golias {

    bool ImGuiContext::Initialize(Window* window) {
        mWindow = window;

        GLFWwindow* nativeWindow = static_cast<GLFWwindow*>(window->GetHandle());

#if defined(GOLIAS_WITH_EDITOR)
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(nativeWindow, /*install_callbacks=*/false);

    #if defined(GOLIAS_PLATFORM_OSX)
        const char* glslVersion = "#version 150";
    #else
        const char* glslVersion = "#version 330";
    #endif

        if (!ImGui_ImplOpenGL3_Init(glslVersion)) {
            return false;
        }
        mInitialized = true;
        return true;
#endif

        mInitialized = false;

        return false;
    }

    void ImGuiContext::Shutdown() {
        if (!mInitialized) {
            return;
        }

#if defined(GOLIAS_WITH_EDITOR)
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        mWindow      = nullptr;
        mInitialized = false;
#endif
    }

    void ImGuiContext::BeginFrame() {
        if (!mInitialized) {
            return;
        }
        
#if defined(GOLIAS_WITH_EDITOR)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
#endif
    }

    void ImGuiContext::EndFrame() {
        if (!mInitialized) {
            return;
        }

#if defined(GOLIAS_WITH_EDITOR)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
    }

} // namespace golias
