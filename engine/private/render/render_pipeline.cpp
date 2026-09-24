#include "render/render_pipeline.h"

#include "render/passes/canvas_pass.h"
#include "render/passes/opaque_geometry_pass.h"
#include "render/passes/post_process_pass.h"
#include "render/passes/shadow_pass.h"
#include "render/passes/skybox_pass.h"
#include "render/passes/sprite_pass.h"
#include "render/passes/transparent_pass.h"
#include "render/passes/unlit_pass.h"

namespace golias {

    bool RenderPipeline::Initialize(RenderTargets* targets) {

        if (!targets) {
            GOLIAS_LOG_ERROR("RenderTargets is invalid.");
            return false;
        }

        auto geometryPass    = std::make_shared<OpaqueGeometryPass>();
        auto shadowPass      = std::make_shared<ShadowPass>(*targets, *geometryPass);
        auto skyboxPass      = std::make_shared<SkyboxPass>();
        auto transparentPass = std::make_shared<TransparentPass>(*geometryPass);
        auto unlitPass       = std::make_shared<UnlitPass>();
        auto spritePass      = std::make_shared<SpritePass>();
        auto postProcessPass = std::make_shared<PostProcessPass>();
        auto canvasPass      = std::make_shared<CanvasPass>();

        AddPass(shadowPass);
        AddPass(skyboxPass);
        AddPass(geometryPass);
        AddPass(transparentPass);
        AddPass(unlitPass);
        AddPass(spritePass);
        AddPass(postProcessPass);
        AddPass(canvasPass);

        return Setup();
    }

    void RenderPipeline::AddPass(const Ref<RenderPass>& pass) {
        mPasses.push_back(pass);
    }

    bool RenderPipeline::Setup() {
        for (const Ref<RenderPass>& pass : mPasses) {
            if (!pass->Setup()) {
                GOLIAS_LOG_ERROR("Render pass '%s' failed to set up", pass->GetName());
                return false;
            }
        }

        mIsSetup = true;
        return true;
    }

    void RenderPipeline::Execute(FrameContext& ctx) {
        GOLIAS_ASSERT_MSG(mIsSetup, "Called before Setup() succeeded");

        for (const Ref<RenderPass>& pass : mPasses) {
            pass->Execute(ctx);
        }
    }

} // namespace golias
