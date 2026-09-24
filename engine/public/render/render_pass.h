#pragma once
#include "graphics/framebuffer.h"
#include "graphics/graphics_device.h"
#include "graphics/texture_2d.h"
#include "graphics/texture_2d_array.h"
#include "math/frustum.h"
#include "render/command_queue.h"

namespace golias {

    /// @brief  Everything a RenderPass needs for one camera's frame.
    struct FrameContext {
        const CameraCommand& Camera;
        Frustum Frustum;

        /// @brief  Filled by OpaqueGeometryPass (single frustum-cull pass produces both lists).
        std::vector<const RenderCommand*> Opaque      = {};
        std::vector<const RenderCommand*> Transparent = {};

        RenderTargets* RenderTargets = nullptr;

        // Ref<Texture2D> HdrColorTexture = nullptr;
        // Ref<Texture2D> HdrDepthTexture = nullptr;

        // Ref<Texture2D> LdrColorTexture = nullptr;

        /// @brief  Populated by ShadowPass when a shadow-casting light is active this frame; null otherwise.
        Ref<Texture2DArray> ShadowTexture = nullptr;

        std::array<glm::mat4, kMaxShadowCascades> ShadowMatrices = {};
        std::array<float, kMaxShadowCascades> ShadowSplits       = {};

        const std::vector<LightCommand>& Lights;
        const std::vector<RenderCommand>& Commands;
        const std::vector<SpriteRenderCommand>& Sprites;
        const std::vector<RenderCanvasCommand>& Canvas;
        const std::vector<UnlitCommand>& Unlit;

        GraphicsDevice& Device;
    };

    class RenderPass {
    public:
        virtual ~RenderPass() = default;

        /// @brief  One-time resource creation only
        virtual bool Setup() {
            return true;
        }

        /// @brief  Bind-and-draw only.
        virtual void Execute(FrameContext& ctx) = 0;

        virtual const char* GetName() const = 0;
    };

} // namespace golias
