#pragma once
#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Shader;
    class Mesh;

    /// @brief  Draws queued 3D debug/gizmo lines with the unlit shader.
    class UnlitPass : public RenderPass {
    public:
        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "UnlitPass";
        }

    private:
        Ref<Shader> mUnlitShader = nullptr;
        Ref<Mesh> mUnlitMesh     = nullptr;

        FrameParameter mFrameParams;
    };

} // namespace golias
