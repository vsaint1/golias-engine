#pragma once

#include "core/graphics/framebuffer.h"
#include "core/graphics/material.h"
#include "core/graphics/mesh.h"
#include "core/graphics/shader.h"
#include "core/graphics/texture_2d.h"
#include "core/graphics/texture_cubemap.h"
#include "physics/3d/physics_debug_drawer.h"
#include <SDL3/SDL.h>


namespace golias {

    enum ERenderingDeviceType {
        COMPATIBILITY,
        FORWARD_PLUS,
    };


    class RenderingDevice {
    public:
        RenderingDevice() = default;

        virtual ~RenderingDevice() = default;

        virtual bool Initialize(SDL_Window* sdl_window) = 0;

        virtual void BindShader(Shader* shader)       = 0;
        virtual void BindMesh(Mesh* mesh)             = 0;
        virtual void UnbindMesh(Mesh* mesh)           = 0;
        virtual void BindMaterial(Material* material) = 0;

        virtual std::shared_ptr<Shader> GetDefaultShader3D() const     = 0;
        virtual std::shared_ptr<Shader> GetDefaultShader2D() const     = 0;
        virtual std::shared_ptr<Shader> GetDefaultShaderCanvas() const = 0;

        virtual std::shared_ptr<Texture2D> CreateTextureFromFile(const std::string_view pFilePath)                       = 0;
        virtual std::shared_ptr<Texture2D> CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) = 0;

        virtual std::shared_ptr<Shader> CreateShaderFromFile(const std::string_view pFilePath)                                     = 0;
        virtual std::shared_ptr<Shader> CreateShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;

        virtual Buffer CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) = 0;

        virtual std::shared_ptr<Mesh> CreateMesh() = 0;
        virtual std::shared_ptr<Mesh>
            CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) = 0;

        virtual PhysicsDebugDrawer* GetPhysicsDebugDrawer() = 0;

        virtual void Clear(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) = 0;

        virtual void DrawMesh(Mesh* mesh) = 0;

        virtual void SwapChain() = 0;

        virtual std::shared_ptr<Framebuffer> CreateFramebuffer(const FramebufferSpec& specification) = 0;

        virtual std::shared_ptr<Shader> GetDefaultShadowMapShader() const = 0;
        virtual void BeginShadowPass()                                    = 0;
        virtual void EndShadowPass()                                      = 0;

        const Viewport& GetViewport() const;

        virtual void SetViewport(const Viewport& vp) = 0;

        virtual std::shared_ptr<Framebuffer> GetDefaultFramebuffer() = 0;
        virtual std::shared_ptr<Framebuffer> GetDefaultShadowMapFramebuffer() = 0;

        virtual std::shared_ptr<TextureCubemap> CreateCubemapFromFaces(const std::array<std::string, 6>& faces) = 0;
        virtual std::shared_ptr<TextureCubemap> CreateCubemapFromCross(const std::string& crossPath) = 0;
        virtual std::shared_ptr<Shader> GetDefaultSkyboxShader() const = 0;

        virtual std::shared_ptr<Texture2D> GetWhiteTexture2D() const = 0;
        virtual std::shared_ptr<Texture2D> GetNormalTexture2D() const = 0;

    protected:
        glm::vec4 clear_color                              = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
        SDL_Window* window                                 = nullptr;
        std::unique_ptr<PhysicsDebugDrawer> physicsDebug3D = nullptr;

        std::shared_ptr<Texture2D> whiteTexture2D                     = nullptr;
        std::shared_ptr<Texture2D> normalTexture2D                     = nullptr;

        Viewport viewport                                      = Viewport();
        Scissor scissor = Scissor();
    };
}; // namespace golias
