#include "render/passes/unlit_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/material.h"
#include "render/frame_params.h"
#include "render/mesh.h"
#include "render/render_targets.h"

namespace golias {

    bool UnlitPass::Setup() {
        mUnlitShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/unlit.gshader");
        if (!mUnlitShader) {
            GOLIAS_LOG_ERROR("Failed to create unlit shader program");
            return false;
        }
        mUnlitShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);

        VertexLayout layout;
        layout.Elements = {
            {0, VertexFormat::Float3, 0                },
            {1, VertexFormat::Float4, 3 * sizeof(float)},
            {2, VertexFormat::Float2, 7 * sizeof(float)},
        };

        layout.Stride = 9 * sizeof(float);

        mUnlitMesh = std::make_shared<Mesh>(layout, std::vector<float>{}, std::vector<uint32_t>{});
        return true;
    }

    void UnlitPass::Execute(FrameContext& ctx) {
        GraphicsDevice& device = ctx.Device;

        if (!ctx.Unlit.empty()) {
            device.SetDepthTestEnabled(true);
            device.SetViewport(ctx.Camera.Viewport);
            mFrameParams.Update(device, BuildGpuFrame(ctx.Camera, ctx.ShadowMatrices, ctx.ShadowSplits));

            mUnlitShader->Bind();
            Ref<Texture2D> whiteTex = Engine::GetInstance().GetAssetManager().AcquireWhiteTexture();

            for (const UnlitCommand& command : ctx.Unlit) {
                if (command.Vertices.empty()) {
                    continue;
                }

                if (command.Material) {
                    if (!command.Material->GetShader()) {
                        continue;
                    }

                    command.Material->Bind();
                } else {
                    mUnlitShader->Bind();
                    mUnlitShader->SetTexture(TextureSlots::MainTexture, command.Texture ? command.Texture : whiteTex.get());
                }

                mUnlitMesh->Update(command.Vertices);

                mUnlitMesh->Bind();
                mUnlitMesh->Draw(command.Primitive);
                mUnlitMesh->Unbind();
            }

            device.SetBlendMode(BlendMode::None);
            device.SetDepthWriteEnabled(true);
        }

        ctx.RenderTargets->GetHdrFramebuffer()->Bind();
    }

} // namespace golias
