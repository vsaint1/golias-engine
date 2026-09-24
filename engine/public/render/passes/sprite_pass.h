#pragma once
#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Shader;
    class Material;
    class Mesh;

    /// @brief  Draws queued 2D sprite commands with the default 2D material.
    class SpritePass : public RenderPass {
    public:
        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "SpritePass";
        }

    private:
        Ref<Shader> mDefaultSpriteShader     = nullptr;
        Ref<Material> mDefaultSpriteMaterial = nullptr;
        Ref<Mesh> mQuadMesh                  = nullptr;

        ObjectParameter mObjectParams;
        MaterialParameter mMaterialParams;
    };

} // namespace golias
