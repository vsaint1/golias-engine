#pragma once
#include "graphics/framebuffer.h"
#include "graphics/graphics_device.h"
#include "graphics/texture_2d_array.h"
#include "math/frustum.h"
#include "render/csm.h"

namespace golias {

    class Mesh;
    class Material;
    class Texture2D;

    enum class Tonemap : int {
        None     = 0, // Only applies gamma correction
        ACES     = 1, // ACES filmic tonemapping (approximation)
        Reinhard = 2, // Reinhard tonemapping
    };


    struct RenderCommand {
        Mesh* Mesh         = nullptr;
        Material* Material = nullptr;
        glm::mat4 Model    = glm::mat4(1.0f);
    };

    struct CanvasBatch {
        Texture* Texture    = nullptr;
        uint32_t IndexCount = 0;
    };

    struct RenderCanvasCommand {
        Mesh* Mesh                       = nullptr;
        std::vector<CanvasBatch> Batches = {};
        Viewport Viewport                = {0, 0, 800, 600};
    };


    struct RenderCommand2D {
        Texture* Texture       = nullptr;
        glm::vec4 Color        = glm::vec4(1.0f);
        glm::mat4 Model        = glm::mat4(1.0f);
        glm::vec2 Size         = glm::vec2(100.0f);
        glm::vec2 Pivot        = glm::vec2(0.5f);
        glm::vec2 LowerLeftUV  = glm::vec2(0.0f);
        glm::vec2 UpperRightUV = glm::vec2(1.0f);
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
        glm::mat4 View           = glm::mat4(1.0f);
        glm::mat4 Projection     = glm::mat4(1.0f);
        glm::vec3 CameraPosition = glm::vec3(0.0f);

        /// @brief  Orthographic view matrix for the camera.
        glm::mat4 Ortho = glm::mat4(1.0f);

        float NearPlane               = 0.1f;
        float FarPlane                = 100.0f;
        Viewport Viewport             = {0, 0, 800, 600};
        CascadedShadowMapDesc Shadows = {};
        // RenderTarget* Target = nullptr; // nullptr = default backbuffer
        // uint32_t CullMask    = 0xFFFFFFFF; // which layers this camera renders
        // bool ClearColor      = true;
        // bool ClearDepth      = true;
    };

    class CommandQueue {
    public:
        CommandQueue();
        ~CommandQueue();

        bool Initialize();

        void Submit(const RenderCommand2D& command);
        void Submit(const RenderCommand& command);
        void Submit(const CameraCommand& command);
        void Submit(const LightCommand& command);
        void Submit(const RenderCanvasCommand& command);

        void BeginFrame();

        void Execute();

        void EndFrame();


    private:
        /// @brief Renders the shadow cascades for the given light and camera.
        void RenderShadowCascades(const CameraCommand& cameraCommand, const LightCommand& light);

        /// @brief Renders the post-processing effects for the given camera.
        void RenderPostProcess(const CameraCommand& cameraCommand);

        /// @brief Ensures that the HDR render targets are created and match the given viewport.
        bool EnsureHdrTargets(const Viewport& viewport);

        /// @brief Updates and binds the per-frame lighting uniform buffer.
        void UpdateLightingBuffer();

        /// @brief Splits mCommands into frustum-culled opaque/transparent lists.
        void CategorizeRenderCommands(const Frustum& frustum,
                                      std::vector<const RenderCommand*>& outOpaque,
                                      std::vector<const RenderCommand*>& outTransparent) const;

        /// @brief Binds material params (incl. shadow data) and issues the draw call.
        void DrawRenderCommand(const RenderCommand& command, const CameraCommand& cameraCommand);

        /// @brief Draws opaque geometry (depth-tested, order-independent).
        void RenderGeometry(const CameraCommand& cameraCommand, const std::vector<const RenderCommand*>& opaque);

        /// @brief Sorts and draws transparent geometry back-to-front.
        void RenderTransparent(const CameraCommand& cameraCommand, std::vector<const RenderCommand*>& transparent);

        /// @brief Draws queued sprite/2D commands with the default 2D material.
        void RenderSprites(const CameraCommand& cameraCommand);

        /// @brief Draws queued UI canvas commands (batched by texture).
        void RenderCanvas(const CameraCommand& cameraCommand);


        std::vector<RenderCommand> mCommands             = {};
        std::vector<CameraCommand> mCameraCommands       = {};
        std::vector<LightCommand> mLightCommands         = {};
        std::vector<RenderCommand2D> mCommands2D         = {};
        std::vector<RenderCanvasCommand> mCanvasCommands = {};

        Ref<Buffer> mLightingBuffer = nullptr;

        Ref<Shader> mDefault2DShader   = nullptr;
        Ref<Shader> mDefaultUIShader   = nullptr;
        Ref<Shader> mFxaaShader        = nullptr;
        Ref<Shader> mPostProcessShader = nullptr;

        Ref<Material> mDefault2DMaterial = nullptr;
        Ref<Material> mDefaultUIMaterial = nullptr;

        Ref<Mesh> mQuadMesh       = nullptr;
        Ref<Mesh> mFullscreenQuad = nullptr;

        Ref<Framebuffer> mHdrFramebuffer = nullptr;
        Ref<Texture2D> mHdrColorTexture = nullptr;
        Ref<Texture2DArray> mHdrDepthTexture = nullptr;
        
        Viewport mHdrViewport = {0, 0, 0, 0};

        Ref<Framebuffer> mShadowFramebuffer = nullptr;
        Ref<Texture2DArray> mShadowTexture  = nullptr;
        Ref<Shader> mShadowShader           = nullptr;

        CascadedShadowMap mShadowCsm;

        Tonemap mTonemap = Tonemap::Reinhard;
    };
} // namespace golias
