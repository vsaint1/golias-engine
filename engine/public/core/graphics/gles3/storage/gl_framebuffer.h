#pragma once

#include "core/graphics/framebuffer.h"
#include "core/graphics/texture_2d.h"
#include <memory>
#include <vector>

namespace golias {

    class OpenglFramebuffer : public Framebuffer {
    public:
        OpenglFramebuffer(const FramebufferSpec& specification);
        ~OpenglFramebuffer() override;

        void Bind() override;
        void Unbind() override;
        void Resize(Uint32 width, Uint32 height) override;

        bool IsValid() const override;

        const FramebufferSpec& GetSpecification() const override;

        std::shared_ptr<Texture2D> GetColorAttachment(Uint32 index) const override;
        std::shared_ptr<Texture2D> GetDepthAttachment() const override;

    private:
        void Invalidate();
        void Cleanup();

    private:
        FramebufferSpec spec;
        Uint32 fbo = 0;

        std::vector<std::shared_ptr<Texture2D>> colorAttachments;
        std::shared_ptr<Texture2D> depthAttachment;
    };

} // namespace golias
