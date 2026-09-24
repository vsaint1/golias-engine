#pragma once
#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Shader;
    class Mesh;

    /// @brief  Post-processes the rendered image.
    class PostProcessPass : public RenderPass {
    public:
        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "PostProcessPass";
        }

    private:
        Ref<Shader> mPostProcessShader = nullptr;
        Ref<Shader> mFxaaShader        = nullptr;
        Ref<Mesh> mFullscreenQuad       = nullptr;

        Tonemap mTonemap = Tonemap::Neutral;

        PostProcessParameter mPostProcessParams;
    };

} // namespace golias
