#pragma once
#include "render/render_pass.h"

namespace golias {

    class Shader;
    class Material;

    /// @brief  Draws queued UI canvas commands (batched by texture, clip-rect scissored).
    class CanvasPass : public RenderPass {
    public:
        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "CanvasPass";
        }

    private:
        Ref<Shader> mDefaultUIShader     = nullptr;
        Ref<Material> mDefaultUIMaterial = nullptr;
    };

} // namespace golias
