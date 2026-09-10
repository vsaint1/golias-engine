#pragma once

namespace golias {

    class Window;


    class ImGuiContext {
    public:
        ImGuiContext()          = default;
        virtual ~ImGuiContext() = default;

        ImGuiContext(const ImGuiContext&)            = delete;
        ImGuiContext& operator=(const ImGuiContext&) = delete;

        bool Initialize(Window* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        bool IsInitialized() const {
            return mInitialized;
        }

    private:
        Window* mWindow   = nullptr;
        bool mInitialized = false;
    };

} // namespace golias
