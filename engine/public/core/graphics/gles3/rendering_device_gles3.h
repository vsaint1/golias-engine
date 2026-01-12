#pragma once

#include "core/graphics/gles3/shaders/gl_shader.h"
#include "core/graphics/gles3/storage/gl_mesh.h"
#include "core/graphics/gles3/storage/gl_texture_2d.h"
#include "core/graphics/rendering_device.h"


namespace golias {


    class RenderingDeviceGLES3 final : public RenderingDevice {
    public:
        RenderingDeviceGLES3() = default;

        ~RenderingDeviceGLES3();

        bool Initialize(SDL_Window* sdl_window) override;

        void BindShader(Shader* shader) override;
        void BindMesh(Mesh* mesh) override;
        void UnbindMesh(Mesh* mesh) override;
        void BindMaterial(Material* material) override;

        std::shared_ptr<Shader> GetDefaultShader3D() const;
        std::shared_ptr<Shader> GetDefaultShader2D() const;
        std::shared_ptr<Shader> GetDefaultShaderCanvas() const;
        std::shared_ptr<Shader> GetDefaultShadowMapShader() const override;

        std::shared_ptr<Texture2D> CreateTextureFromFile(const std::string_view pFilePath) override;
        std::shared_ptr<Texture2D> CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) override;

        std::shared_ptr<Shader> CreateShaderFromFile(const std::string_view pFilePath) override;
        std::shared_ptr<Shader> CreateShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) override;


        std::shared_ptr<Mesh> CreateMesh() override;
        std::shared_ptr<Mesh> CreateMeshFromData(const VertexLayout& layout,
                                                 const std::vector<float>& vertices,
                                                 const std::vector<uint32_t>& indices) override;

        Buffer CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) override;

        std::shared_ptr<Framebuffer> CreateFramebuffer(const FramebufferSpec& speficiation) override;

        PhysicsDebugDrawer* GetPhysicsDebugDrawer() override;

        void Clear(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) override;

        void DrawMesh(Mesh* mesh);

        void SwapChain() override;

        void SetViewport(const Viewport& vp) override;

        void BeginShadowPass() override;
        void EndShadowPass() override;

        std::shared_ptr<Framebuffer> GetDefaultFramebuffer() override {
            return nullptr;
        }

        std::shared_ptr<Framebuffer> GetDefaultShadowMapFramebuffer() override {
            return shadowFBO;
        }

        // Skybox and IBL
        Uint32 CreateCubemapFromFiles(const std::array<std::string, 6>& faces) override;
        Uint32 CreateCubemapFromCross(const std::string& crossPath) override;
        std::shared_ptr<Shader> GetDefaultSkyboxShader() const override;
        std::shared_ptr<Mesh> GetSkyboxMesh() override;
        Uint32 GetDefaultSkyboxCubemap() const override;

    private:
        SDL_GLContext gl_context = nullptr;

        std::shared_ptr<Shader> default_shader_3d     = nullptr;
        std::shared_ptr<Shader> default_shader_2d     = nullptr;
        std::shared_ptr<Shader> default_shader_canvas = nullptr;
        std::shared_ptr<Shader> default_shader_csm    = nullptr;
        std::shared_ptr<Shader> skybox_shader         = nullptr;


        std::shared_ptr<Framebuffer> shadowFBO = nullptr;
        std::shared_ptr<Mesh> skybox_mesh      = nullptr;
        Uint32 default_skybox_cubemap          = 0;

        bool CreateDefaultShaders();
        bool CreateDefaultFramebuffers();
        bool CreateSkyboxShader();
        bool CreateDefaultSkybox();
    };
}; // namespace golias
