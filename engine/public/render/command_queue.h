#pragma once
#include "graphics/gpu_types.h"
#include "graphics/render_types.h"
#include "math/aabb.h"
#include "render/csm.h"
#include "render/shader_parameter.h"

namespace golias {

    class Mesh;
    class Material;
    class Texture;
    class Texture2D;
    class TextureCube;
    class Query;
    class Buffer;
    class RenderPipeline;
    class RenderTargets;

    /// @brief  Maximum number of instances drawn draw call.
    inline constexpr size_t kMaxInstancesPerBatch = 65535;

    /// @brief  Maximum number of joints used for skeletal animation.
    inline constexpr uint32_t kMaxJoints = 256;

    enum class Tonemap : int {
        None     = 0, // Only applies gamma correction
        ACES     = 1, // ACES filmic tonemapping (approximation)
        Reinhard = 2, // Reinhard tonemapping
        Neutral   = 3, // Neutral tonemapping
    };


    struct RenderCommand {
        Mesh* Mesh         = nullptr;
        Material* Material = nullptr;
        glm::mat4 Model    = glm::mat4(1.0f);

        /// @brief  Skin joint matrices (null for static meshes).
        const glm::mat4* JointMatrices = nullptr;
        uint32_t JointCount            = 0;

        /// @brief  Identity of the joint matrices owner.
        const void* JointKey  = nullptr;
        uint64_t JointVersion = 0;

        /// @brief  Optional world-space bounding box used for skinned-mesh culling.
        const AABB* WorldBounds = nullptr;
    };

    /// @brief  Per-instance data streamed to the GPU for instanced draw calls.
    struct InstanceData {
        glm::mat4 Model = glm::mat4(1.0f);
        glm::vec4 Color = glm::vec4(1.0f);
    };

    static_assert(sizeof(InstanceData) == sizeof(glm::mat4) + sizeof(glm::vec4),
                  "InstanceData must remain a tightly packed { matrix, color } pair");

    struct CanvasBatch {
        Texture* Texture    = nullptr;
        uint32_t IndexCount = 0;

        ScissorRect ClipRect = {};
        bool HasClip         = false;
    };

    struct UnlitCommand {
        Texture* Texture = nullptr;
        Material* Material = nullptr;
        std::vector<float> Vertices = {}; // vec3 + vec4 + vec2 (position + color + texcoord)
        PrimitiveType Primitive = PrimitiveType::Triangles;
    };

    struct RenderCanvasCommand {
        Mesh* Mesh                       = nullptr;
        std::vector<CanvasBatch> Batches = {};
        Viewport Viewport                = {0, 0, 800, 600};
    };


    struct SpriteRenderCommand {
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

    /// @brief  Collects per-frame draw commands and drives them through a fixed RenderPipeline of
    ///         RenderPass stages (shadows -> opaque -> transparent -> unlit -> post-process -> sprites
    ///         -> canvas). Submit(...) is pure data collection; all actual GPU work lives in the passes.
    class CommandQueue {
    public:
        CommandQueue();
        ~CommandQueue();

        bool Initialize();

        void Submit(const SpriteRenderCommand& command);
        void Submit(const RenderCommand& command);
        void Submit(const CameraCommand& command);
        void Submit(const LightCommand& command);
        void Submit(const RenderCanvasCommand& command);
        void Submit(const UnlitCommand& command);

        void BeginFrame();

        void Execute();

        void EndFrame();

    private:
        std::vector<RenderCommand> mCommands             = {};
        std::vector<CameraCommand> mCameraCommands       = {};
        std::vector<LightCommand> mLightCommands         = {};
        std::vector<SpriteRenderCommand> mSpriteCommands = {};
        std::vector<RenderCanvasCommand> mCanvasCommands = {};
        std::vector<UnlitCommand> mUnlitCommands         = {};

        RenderTargets* mRenderTargets = nullptr;
        RenderPipeline* mPipeline     = nullptr;

        /// @brief  Frame-level renderer state (not pass-owned): the lighting parameter block is uploaded
        ///         once per frame regardless of camera, and the zero-filled default joint buffer is
        ///         rebound once per frame so non-skinned draws never sample a stale per-object skin buffer.
        LightingParameter mLightingParams;
        Ref<Buffer> mDefaultJointBuffer = nullptr;

        Ref<Query> mActiveGpuQuery  = nullptr;
        Ref<Query> mStandbyGpuQuery = nullptr;
    };
} // namespace golias
