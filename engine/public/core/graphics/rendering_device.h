#pragma once

#include "core/application.h"
#include "core/graphics/framebuffer.h"
#include "core/graphics/material.h"
#include "core/graphics/mesh.h"
#include "core/graphics/shader.h"
#include "core/graphics/texture_2d.h"
#include "core/graphics/texture_cubemap.h"
#include "physics/3d/physics_debug_drawer.h"
#include <SDL3/SDL.h>


namespace golias {

    static constexpr int NUM_SHADOW_CASCADES = 4;

    /**
     * @brief Low-level rendering device abstraction
     * 
     * This is a thin API abstraction layer for GPU operations.
     * Supports OpenGL ES 3.0, WebGL 2.0, and can be extended to modern APIs.
     * 
     * Design principles:
     * - Immutable pipeline states (no mid-frame state changes)
     * - Explicit render passes with load/store ops
     * - Low-level resource management only
     * - High-level features (default resources, materials) handled by SceneRenderer
     */
    class RenderingDevice {
    public:
        RenderingDevice()          = default;
        virtual ~RenderingDevice() = default;

        // ============================================================
        // Initialization & Context
        // ============================================================
        virtual bool Initialize(SDL_Window* sdl_window) = 0;
        virtual void SwapChain()                        = 0;

        // ============================================================
        // Pipeline State (Immutable)
        // ============================================================

        /**
         * @brief Apply a complete pipeline state
         * Must be called before any draw calls
         * State remains bound until next ApplyPipelineState call
         */
        virtual void ApplyPipelineState(const PipelineState& state) = 0;

        /**
         * @brief Bind a shader program
         * Shaders are part of the draw state but bound separately for flexibility
         */
        virtual void BindShader(Shader* shader) = 0;

        // ============================================================
        // Render Pass Management
        // ============================================================

        /**
         * @brief Begin a render pass with explicit load/store operations
         * Replaces implicit state changes for better API compatibility
         */
        virtual void BeginRenderPass(const RenderPassBeginInfo& info) = 0;

        /**
         * @brief End the current render pass
         */
        virtual void EndRenderPass() = 0;

        // ============================================================
        // Resource Creation (Low-level only)
        // ============================================================

        // Textures
        virtual std::shared_ptr<Texture2D> CreateTextureFromFile(const std::string_view pFilePath)                       = 0;
        virtual std::shared_ptr<Texture2D> CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) = 0;

        // Cubemaps
        virtual std::shared_ptr<TextureCubemap> CreateCubemapFromFaces(const std::array<std::string, 6>& faces) = 0;
        virtual std::shared_ptr<TextureCubemap> CreateCubemapFromCross(const std::string& crossPath)            = 0;
        virtual std::shared_ptr<TextureCubemap> CreateCubemapProcedural()                                       = 0;

        // Shaders
        virtual std::shared_ptr<Shader> CreateShaderFromFile(const std::string_view pFilePath)                                     = 0;
        virtual std::shared_ptr<Shader> CreateShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;

        // Buffers
        virtual Buffer CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) = 0;

        // Meshes
        virtual std::shared_ptr<Mesh> CreateMesh() = 0;
        virtual std::shared_ptr<Mesh>
            CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) = 0;

        // Framebuffers
        virtual std::shared_ptr<Framebuffer> CreateFramebuffer(const FramebufferSpec& specification) = 0;

        // ============================================================
        // Resource Binding
        // ============================================================

        virtual void BindMesh(Mesh* mesh)                                                                            = 0;
        virtual void UnbindMesh(Mesh* mesh)                                                                          = 0;
        virtual void BindTexture(Shader* shader, std::string_view uniformName, Uint32 slot, Texture2D* texture)      = 0;
        virtual void BindCubemap(Shader* shader, std::string_view uniformName, Uint32 slot, TextureCubemap* texture) = 0;
        virtual void BindMaterial(Material* material)                                                                = 0;

        // ============================================================
        // Draw Commands
        // ============================================================

        virtual void DrawMesh(Mesh* mesh) = 0;

        // ============================================================
        // Dynamic State (Legacy compatibility)
        // ============================================================

        virtual void SetViewport(const Viewport& vp)    = 0;
        virtual void SetScissor(const Scissor& scissor) = 0;

        /**
         * @brief Legacy mutable state setters
         * @deprecated Use PipelineState instead for modern rendering
         * Kept for backwards compatibility during refactoring
         */
        virtual void SetColorWrite(bool red, bool green, bool blue, bool alpha) = 0;
        virtual void SetBlendMode(EBlendMode blendMode)                         = 0;
        virtual void SetDepthComparison(EComparisonFunc func)                   = 0;
        virtual void SetCullMode(ECullMode cullMode)                            = 0;
        virtual void SetDepthWrite(bool enable)                                 = 0;
        virtual void SetDepthTest(bool enable)                                  = 0;

        /**
         * @brief Legacy clear operations
         * @deprecated Use BeginRenderPass with proper load ops
         */
        virtual void ClearColor(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) = 0;
        virtual void ClearBuffer(EClearFlags flags)                                  = 0;

        // ============================================================
        // High-level Resource Access (Will be moved to ResourceManager)
        // ============================================================

        virtual std::shared_ptr<Shader> GetDefaultShader3D() const        = 0;
        virtual std::shared_ptr<Shader> GetDefaultShader2D() const        = 0;
        virtual std::shared_ptr<Shader> GetDefaultShaderCanvas() const    = 0;
        virtual std::shared_ptr<Shader> GetDefaultShadowMapShader() const = 0;
        virtual std::shared_ptr<Shader> GetDefaultSkyboxShader() const    = 0;

        virtual std::shared_ptr<Texture2D> GetWhiteTexture2D() const           = 0;
        virtual std::shared_ptr<Texture2D> GetNormalTexture2D() const          = 0;
        virtual std::shared_ptr<TextureCubemap> GetWhiteTextureCubemap() const = 0;

        virtual std::shared_ptr<Framebuffer> GetDefaultFramebuffer()                                            = 0;
        virtual std::shared_ptr<Framebuffer> GetCascadeShadowMapFramebuffer(int index) = 0;

        virtual PhysicsDebugDrawer* GetPhysicsDebugDrawer() = 0;

        // ============================================================
        // Query
        // ============================================================

        const Viewport& GetViewport() const;
        SDL_Window* GetWindow() const {
            return window;
        }

    protected:
        glm::vec4 clear_color                              = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
        SDL_Window* window                                 = nullptr;
        std::unique_ptr<PhysicsDebugDrawer> physicsDebug3D = nullptr;

        std::shared_ptr<Texture2D> whiteTexture2D           = nullptr;
        std::shared_ptr<Texture2D> normalTexture2D          = nullptr;
        std::shared_ptr<TextureCubemap> whiteTextureCubemap = nullptr;

        Viewport viewport = Viewport();
        Scissor scissor   = Scissor();
    };
}; // namespace golias
