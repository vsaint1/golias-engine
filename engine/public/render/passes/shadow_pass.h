#pragma once
#include "render/csm.h"
#include "render/frame_params.h"
#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Shader;
    class RenderTargets;
    class OpaqueGeometryPass;

    /// @brief  Builds cascaded shadow maps for the first shadow-casting directional light and depth-only draws opaque casters into them.
    class ShadowPass : public RenderPass {
    public:
        ShadowPass(RenderTargets& targets, OpaqueGeometryPass& geometryPass);

        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "ShadowPass";
        }

    private:
        void RenderShadowCascades(FrameContext& ctx, const LightCommand& light);
        void DrawShadowOpaque(FrameContext& ctx, const std::vector<const RenderCommand*>& casters);

        RenderTargets& mTargets;
        OpaqueGeometryPass& mGeometryPass;

        Ref<Shader> mShadowShader = nullptr;
        CascadedShadowMap mShadowCsm;

        FrameParameter mFrameParams;
        ObjectParameter mObjectParams;
        JointParameterCache mJoints;
    };

} // namespace golias
