#include "render/passes/canvas_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/render_stats.h"

namespace golias {

    bool CanvasPass::Setup() {
        mDefaultUIShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/default_ui.gshader");
        if (!mDefaultUIShader) {
            GOLIAS_LOG_ERROR("Failed to create default UI shader program");
            return false;
        }
        mDefaultUIShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);

        mDefaultUIMaterial = std::make_shared<Material>();
        mDefaultUIMaterial->SetShader(mDefaultUIShader);
        mDefaultUIMaterial->SetDepthTestEnabled(false);
        mDefaultUIMaterial->SetBlendMode(BlendMode::Alpha);

        return true;
    }

    void CanvasPass::Execute(FrameContext& ctx) {
        GraphicsDevice& device = ctx.Device;

        if (ctx.Canvas.empty()) {
            device.SetBlendMode(BlendMode::None);
            device.SetDepthTestEnabled(true);
            return;
        }

        for (const auto& command : ctx.Canvas) {
            if (!command.Mesh || command.Batches.empty()) {
                continue;
            }

            command.Mesh->Bind();
            mDefaultUIMaterial->Bind();
            Shader* shader       = mDefaultUIMaterial->GetShader().get();
            uint32_t indexOffset = 0;
            for (const CanvasBatch& batch : command.Batches) {

                if (batch.IndexCount > 0) {
                    FrameStats::RecordCanvasBatch(1);

                    if (batch.HasClip) {

                        ScissorRect scissor = batch.ClipRect.ToScissorRect(command.Viewport);

                        device.SetScissorTestEnabled(true);
                        device.SetScissorRect(scissor);

                    } else {
                        device.SetScissorTestEnabled(false);
                    }

                    Ref<Texture2D> whiteTex = Engine::GetInstance().GetAssetManager().AcquireWhiteTexture();
                    shader->SetTexture(TextureSlots::MainTexture, batch.Texture ? batch.Texture : whiteTex.get());
                    command.Mesh->DrawIndexed(indexOffset, batch.IndexCount);
                }

                indexOffset += batch.IndexCount;
            }

            command.Mesh->Unbind();
        }

        device.SetScissorTestEnabled(false);

        device.SetBlendMode(BlendMode::None);
        device.SetDepthTestEnabled(true);
    }

} // namespace golias
