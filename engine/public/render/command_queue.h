#pragma once
#include "graphics/graphics_device.h"

namespace golias {

    class Mesh;
    class Material;

    struct RenderCommand {
        Mesh* Mesh         = nullptr;
        Material* Material = nullptr;
        glm::mat4 Model    = glm::mat4(1.0f);
    };

    struct CameraCommand {
        glm::mat4 View       = glm::mat4(1.0f);
        glm::mat4 Projection = glm::mat4(1.0f);
        Viewport Viewport     = {0, 0, 800, 600};
        // RenderTarget* Target = nullptr; // nullptr = default backbuffer
        // uint32_t CullMask    = 0xFFFFFFFF; // which layers this camera renders
        // bool ClearColor      = true;
        // bool ClearDepth      = true;
    };

    class CommandQueue {
    public:
        void Submit(const RenderCommand& command);

        void Submit(const CameraCommand& command);

        void BeginFrame();

        void Execute();

        void EndFrame();


    private:
        std::vector<RenderCommand> mCommands       = {};
        std::vector<CameraCommand> mCameraCommands = {};
    };
} // namespace golias
