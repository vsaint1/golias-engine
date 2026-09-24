#pragma once
#include "graphics/framebuffer.h"
#include "graphics/render_types.h"
#include "graphics/texture_2d.h"
#include "graphics/texture_2d_array.h"

namespace golias {

    /// @brief  Owns the HDR/LDR/shadow-map framebuffers and (re)creates them only when the requested size changes.
    class RenderTargets {
    public:
        /// @brief  (Re)creates the HDR scene target and its paired LDR tonemap target when the viewport
        ///         size changes. Returns false if the viewport is degenerate or a framebuffer is incomplete.
        bool EnsureHdrLdr(const Viewport& viewport);

        /// @brief  (Re)creates the CSM shadow framebuffer/texture array when the requested resolution changes.
        bool EnsureShadow(uint32_t resolution, TextureFormat depthFormat);

        Framebuffer* GetHdrFramebuffer() const {
            return mHdrFramebuffer.get();
        }

        Texture2D* GetHdrDepthTexture() const {
            return mHdrDepthTexture.get();
        }

        Texture2D* GetHdrColorTexture() const {
            return mHdrColorTexture.get();
        }

        Framebuffer* GetLdrFramebuffer() const {
            return mLdrFramebuffer.get();
        }

        Texture2D* GetLdrColorTexture() const {
            return mLdrColorTexture.get();
        }

        Framebuffer* GetShadowFramebuffer() const {
            return mShadowFramebuffer.get();
        }

        Ref<Texture2DArray> GetShadowTexture() const {
            return mShadowTexture;
        }

    private:
        bool EnsureLdr(const Viewport& viewport);

        Ref<Framebuffer> mHdrFramebuffer = nullptr;
        Ref<Texture2D> mHdrColorTexture  = nullptr;
        Ref<Texture2D> mHdrDepthTexture  = nullptr;
        Viewport mHdrViewport            = {0, 0, 0, 0};

        Ref<Framebuffer> mLdrFramebuffer = nullptr;
        Ref<Texture2D> mLdrColorTexture  = nullptr;
        Viewport mLdrViewport            = {0, 0, 0, 0};

        Ref<Framebuffer> mShadowFramebuffer = nullptr;
        Ref<Texture2DArray> mShadowTexture  = nullptr;
        uint32_t mShadowResolution          = 0;
    };

} // namespace golias
