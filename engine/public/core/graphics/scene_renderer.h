#pragma once

#include "core/graphics/gles3/rendering_device_gles3.h"

namespace golias {

    class SkeletonAnimationComponent;
    class WorldEnvironmentComponent;

    struct WorldEnvironmentCommand {
        WorldEnvironmentComponent* environmentComponent = nullptr;
    };

    struct DrawCommand {
        Mesh* mesh         = nullptr;
        Material* material = nullptr;
        glm::mat4 modelMatrix;
        SkeletonAnimationComponent* skeletonAnimation = nullptr;
        bool useIBL                                   = true; // Enable/disable IBL for this draw command
    };

    struct DrawCommand2D {
        glm::mat4 modelMatrix;
        Texture2D* texture     = nullptr;
        glm::vec4 color        = glm::vec4(1.0f);
        glm::vec2 size         = glm::vec2(32.0f, 32.0f);
        glm::vec2 lowerLeftUV  = glm::vec2(0.0f, 0.0f);
        glm::vec2 upperRightUV = glm::vec2(1.0f, 1.0f);
        glm::vec2 pivot        = glm::vec2(0.5f, 0.5f);
    };

    struct CameraCommand {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 orthographicMatrix;
        glm::vec3 position;
    };

    struct DirectionalLightCommand {
        glm::vec3 direction = glm::vec3(0.5f, -1.0f, 0.3f);
        glm::vec3 color     = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity     = 1.0f;
        bool castShadows    = true;
        glm::mat4 lightSpaceMatrix;
    };

    struct PointLightCommand {
        glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec3 color    = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity    = 1.0f;
        float range        = 10.0f; // Maximum distance of light influence
        float constant     = 1.0f; // Constant attenuation term
        float linear       = 0.09f; // Linear attenuation term
        float quadratic    = 0.032f; // Quadratic attenuation term
        bool castShadows   = false;
    };

    struct SpotLightCommand {
        glm::vec3 position   = glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec3 direction  = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 color      = glm::vec3(1.0f, 1.0f, 1.0f);
        float intensity      = 1.0f;
        float range          = 10.0f; // Maximum distance of light influence
        float innerConeAngle = 12.5f; // Inner cone angle in degrees
        float outerConeAngle = 17.5f; // Outer cone angle in degrees
        float constant       = 1.0f; // Constant attenuation term
        float linear         = 0.09f; // Linear attenuation term
        float quadratic      = 0.032f; // Quadratic attenuation term
        bool castShadows     = false;
    };

    struct CanvasBatch {
        Texture2D* texture = nullptr;
        Uint32 indexCount  = 0;
    };

    struct ScreenCanvasCommand {
        Mesh* mesh = nullptr;
        std::vector<CanvasBatch> batches;
    };

    struct WorldCanvasCommand {
        Mesh* mesh = nullptr;
        std::vector<CanvasBatch> batches;
        glm::mat4 modelMatrix;
        float scale = 0.01f; // Pixels -> World units
    };

    class SceneRenderer {
    public:
        bool Initialize(SDL_Window* pWindow, ERenderingDeviceType deviceType);

        void Submit(const DrawCommand& command);
        void Submit(const DrawCommand2D& command);
        void Submit(const ScreenCanvasCommand& command);
        void Submit(const WorldCanvasCommand& command);
        void Submit(const DirectionalLightCommand& command);
        void Submit(const PointLightCommand& command);
        void Submit(const SpotLightCommand& command);
        void Submit(const WorldEnvironmentCommand& command);

        void Clear(const glm::vec4& color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));

        void Draw(const CameraCommand& camera);

        void BeginFrame(const glm::vec4& color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        void EndFrame();
        void Present();

        RenderingDevice* GetRenderingDevice() const;

        ~SceneRenderer();

    private:
        WorldEnvironmentCommand world_environment_command;
        std::vector<DrawCommand> command_queue;
        std::vector<DrawCommand2D> command_queue_2d;

        std::vector<DirectionalLightCommand> directional_lights;
        std::vector<PointLightCommand> point_lights;
        std::vector<SpotLightCommand> spot_lights;

        std::vector<ScreenCanvasCommand> canvas_commands;
        std::vector<WorldCanvasCommand> world_canvas_commands;

        std::shared_ptr<Mesh> quad        = nullptr;
        RenderingDevice* rendering_device = nullptr;

        struct RenderContext {
            CameraCommand camera;
            bool shadowsEnabled = true;
        } renderContext{};

        void ShadowPass();
        void GeometryOpaquePass(const std::vector<DrawCommand>& opaqueCommands);
        void SkyboxPass();
        void GeometryTransparentPass(const std::vector<DrawCommand>& transparentCommands);
        void WorldCanvasPass();
        void Sprite2DPass();
        void ScreenCanvasPass();


        void RenderObject(const DrawCommand& command);
        void CalculateLightSpaceMatrices();
        void SetupMaterialUniforms(const DrawCommand& command);
        void SetupLightingUniforms(Shader* shader);
        void SetupShadowUniforms(Shader* shader);
        void SetupIBLUniforms(Shader* shader, const DrawCommand& command);
    };

}; // namespace golias