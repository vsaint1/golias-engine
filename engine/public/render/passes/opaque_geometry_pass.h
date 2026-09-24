#pragma once
#include "render/frame_params.h"
#include "render/render_pass.h"
#include "render/shader_parameter.h"

namespace golias {

    class Shader;
    class Buffer;
    class Mesh;

    /// @brief  Frustum-culls draw commands into opaque/transparent sets and instances+draws the opaque half.
    class OpaqueGeometryPass : public RenderPass {
    public:
        bool Setup() override;
        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "OpaqueGeometryPass";
        }

        /// @brief  Shared with TransparentPass/ShadowPass: binds material params and issues the draw call.
        void DrawRenderCommand(const RenderCommand& command,
                               FrameContext& ctx,
                               uint32_t instanceCount           = 0,
                               const InstanceData* instanceData = nullptr);

        /// @brief  Shared with ShadowPass: streams instance data into the shared instance buffer in
        ///         kMaxInstancesPerBatch-sized chunks and issues one instanced draw call per chunk.
        void DrawInstancedBatches(GraphicsDevice& device, Mesh* mesh, const InstanceData* instanceData, uint32_t instanceCount);

        Shader* GetDefaultShader() const {
            return mDefaultShader.get();
        }

    private:
        void CategorizeRenderCommands(FrameContext& ctx);
        void RenderInstanced(FrameContext& ctx);

        Ref<Shader> mDefaultShader  = nullptr;
        Ref<Buffer> mInstanceBuffer = nullptr;

        FrameParameter mFrameParams;
        ObjectParameter mObjectParams;
        MaterialParameter mMaterialParams;
        JointParameterCache mJoints;
    };

} // namespace golias
