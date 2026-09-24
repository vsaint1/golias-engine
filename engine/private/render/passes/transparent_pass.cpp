#include "render/passes/transparent_pass.h"

#include "core/engine.h"
#include "render/mesh.h"
#include "render/passes/opaque_geometry_pass.h"

namespace golias {

    TransparentPass::TransparentPass(OpaqueGeometryPass& geometryPass) : mGeometryPass(geometryPass) {
    }

    void TransparentPass::Execute(FrameContext& ctx) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        // Sort back-to-front (furthest first).
        const glm::vec3 cameraPosition = ctx.Camera.CameraPosition;
        std::sort(ctx.Transparent.begin(), ctx.Transparent.end(), [&](const RenderCommand* a, const RenderCommand* b) {
            const glm::vec3 centerA = glm::vec3(a->Model * glm::vec4(a->Mesh->GetAABB().GetCenter(), 1.0f));
            const glm::vec3 centerB = glm::vec3(b->Model * glm::vec4(b->Mesh->GetAABB().GetCenter(), 1.0f));
            return glm::distance(cameraPosition, centerA) > glm::distance(cameraPosition, centerB);
        });

        device.SetDepthWriteEnabled(false);

        for (const RenderCommand* command : ctx.Transparent) {
            mGeometryPass.DrawRenderCommand(*command, ctx);
        }

        device.SetDepthWriteEnabled(true);
    }

} // namespace golias
