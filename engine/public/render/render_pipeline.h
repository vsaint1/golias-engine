#pragma once
#include "render/render_pass.h"

namespace golias {

    class RenderTargets;

    /// @brief  Owns the fixed sequence of RenderPass objects and drives them, in order, once per frame.
    class RenderPipeline {
    public:
        bool Initialize(RenderTargets* targets);

        void Execute(FrameContext& ctx);

    private:
        void AddPass(const Ref<RenderPass>& pass);

        bool Setup();

        std::vector<Ref<RenderPass>> mPasses = {};
        bool mIsSetup                        = false;
    };

} // namespace golias
