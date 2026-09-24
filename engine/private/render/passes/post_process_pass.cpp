#include "render/passes/post_process_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/mesh.h"
#include "render/render_targets.h"

namespace golias {

    bool PostProcessPass::Setup() {
        mPostProcessShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/postprocess.gshader");
        if (!mPostProcessShader) {
            GOLIAS_LOG_ERROR("Failed to create post-process shader program");
            return false;
        }

        mFxaaShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/fxaa.gshader");
        if (!mFxaaShader) {
            GOLIAS_LOG_ERROR("Failed to create FXAA shader program");
            return false;
        }

        mPostProcessShader->SetUniformBlockBinding(GpuLayout::PostProcessBlock, GpuLayout::PostProcessBinding);
        mFxaaShader->SetUniformBlockBinding(GpuLayout::PostProcessBlock, GpuLayout::PostProcessBinding);

        // Full screen quad in normalized device coordinates (NDC)
        // TODO: We can move this to shader
        // clang-format off
        const std::vector<float> vertices = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
        };
        // clang-format on

        const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

        VertexLayout layout;
        layout.Elements = {
            {0, VertexFormat::Float2, 0                },
            {1, VertexFormat::Float2, 2 * sizeof(float)},
        };

        layout.Stride   = 4 * sizeof(float);
        mFullscreenQuad = std::make_shared<Mesh>(layout, vertices, indices);

        return true;
    }

    void PostProcessPass::Execute(FrameContext& ctx) {
        GraphicsDevice& device = ctx.Device;

        if (!mPostProcessShader || !mFxaaShader || !mFullscreenQuad || !ctx.RenderTargets) {
            return;
        }

        device.SetDepthTestEnabled(false);
        device.SetBlendMode(BlendMode::None);

        mFullscreenQuad->Bind();

        ctx.RenderTargets->GetLdrFramebuffer()->Bind();
        device.SetViewport(ctx.Camera.Viewport);

        mPostProcessShader->Bind();
        mPostProcessShader->SetTexture(TextureSlots::MainTexture, ctx.RenderTargets->GetHdrColorTexture());
        mPostProcessParams.Update(device, {
            .Exposure = 1.0f,
            .Tonemap  = static_cast<int>(mTonemap),
        });

        mFullscreenQuad->Draw();
        ctx.RenderTargets->GetLdrFramebuffer()->Unbind();

        device.SetViewport(ctx.Camera.Viewport);

        mFxaaShader->Bind();
        mFxaaShader->SetTexture(TextureSlots::MainTexture, ctx.RenderTargets->GetLdrColorTexture());
        mPostProcessParams.Update(device, {
            .TexelSizeX       = 1.0f / static_cast<float>(ctx.RenderTargets->GetLdrColorTexture()->GetDesc().Width),
            .TexelSizeY       = 1.0f / static_cast<float>(ctx.RenderTargets->GetLdrColorTexture()->GetDesc().Height),
            .SubpixelQuality  = 0.75f,
            .EdgeThreshold    = 0.25f,
            .EdgeThresholdMin = 0.0625f,
        });

        mFullscreenQuad->Draw();
        mFullscreenQuad->Unbind();
    }

} // namespace golias
