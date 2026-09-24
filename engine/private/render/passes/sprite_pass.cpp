#include "render/passes/sprite_pass.h"

#include "core/engine.h"
#include "graphics/shader.h"
#include "render/frame_params.h"
#include "render/material.h"
#include "render/mesh.h"

namespace golias {

    bool SpritePass::Setup() {
        mQuadMesh = Mesh::CreateQuad();

        mDefaultSpriteShader = Engine::GetInstance().GetAssetManager().Load<Shader>("golias/shaders/default_2d.gshader");
        if (!mDefaultSpriteShader) {
            GOLIAS_LOG_ERROR("Failed to create default 2D shader program");
            return false;
        }
        mDefaultSpriteShader->SetUniformBlockBinding(GpuLayout::FrameBlock, GpuLayout::FrameBinding);
        mDefaultSpriteShader->SetUniformBlockBinding(GpuLayout::ObjectBlock, GpuLayout::ObjectBinding);
        mDefaultSpriteShader->SetUniformBlockBinding(GpuLayout::MaterialBlock, GpuLayout::MaterialBinding);

        mDefaultSpriteMaterial = std::make_shared<Material>();
        mDefaultSpriteMaterial->SetShader(mDefaultSpriteShader);
        mDefaultSpriteMaterial->SetDepthTestEnabled(false);
        mDefaultSpriteMaterial->SetBlendMode(BlendMode::Alpha);

        return true;
    }

    void SpritePass::Execute(FrameContext& ctx) {
        GraphicsDevice& device = ctx.Device;

        // Sprites/canvas are unlit 2D overlays drawn on top of the tonemapped result.
        device.SetDepthTestEnabled(false);
        device.SetBlendMode(BlendMode::Alpha);

        if (ctx.Sprites.empty()) {
            return;
        }

        mQuadMesh->Bind();

        for (const auto& command : ctx.Sprites) {
            mDefaultSpriteMaterial->Bind();
            Shader* shader = mDefaultSpriteMaterial->GetShader().get();
            mObjectParams.Update(
                ctx.Device, BuildGpuObject(command.Model, 0, 0, glm::vec4(command.Pivot, command.Size), glm::vec4(command.LowerLeftUV, command.UpperRightUV)));
            mMaterialParams.Update(ctx.Device, BuildGpuMaterial(command.Color));
            shader->SetTexture(TextureSlots::MainTexture, command.Texture);
            mQuadMesh->Draw();
        }

        mQuadMesh->Unbind();
    }

} // namespace golias
