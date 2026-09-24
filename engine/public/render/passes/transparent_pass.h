#pragma once
#include "render/render_pass.h"

namespace golias {

    class OpaqueGeometryPass;

    /// @brief  Sorts transparent geometry back-to-front and draws it with depth-write disabled. 
    class TransparentPass : public RenderPass {
    public:
        explicit TransparentPass(OpaqueGeometryPass& geometryPass);

        void Execute(FrameContext& ctx) override;

        const char* GetName() const override {
            return "TransparentPass";
        }

    private:
        OpaqueGeometryPass& mGeometryPass;
    };

} // namespace golias
