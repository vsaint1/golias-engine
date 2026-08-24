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

    struct LightCommand {
        glm::vec3 Position  = glm::vec3(0.0f);
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 Color     = glm::vec3(1.0f);
        float Intensity     = 1.0f;
        float Range         = 10.0f;
        float SpotAngle     = 45.0f;
        int Type            = 0; // 0 = directional, 1 = point, 2 = spot
        bool IsShadowCaster = false;
    };

    struct CameraCommand {
        glm::mat4 View       = glm::mat4(1.0f);
        glm::mat4 Projection = glm::mat4(1.0f);
        glm::vec3 CameraPosition = glm::vec3(0.0f);
        Viewport Viewport    = {0, 0, 800, 600};
        // RenderTarget* Target = nullptr; // nullptr = default backbuffer
        // uint32_t CullMask    = 0xFFFFFFFF; // which layers this camera renders
        // bool ClearColor      = true;
        // bool ClearDepth      = true;
    };

    class CommandQueue {
    public:
        ~CommandQueue();

        void Submit(const RenderCommand& command);

        void Submit(const CameraCommand& command);

        void Submit(const LightCommand& command);

        void BeginFrame();

        void Execute();

        void EndFrame();


    private:
        std::vector<RenderCommand> mCommands       = {};
        std::vector<CameraCommand> mCameraCommands = {};
        std::vector<LightCommand> mLightCommands = {};
        GLuint mLightingBuffer = 0;
    };
} // namespace golias
