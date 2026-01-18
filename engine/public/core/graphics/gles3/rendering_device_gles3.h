#pragma once

#include "core/graphics/gles3/shaders/gl_shader.h"
#include "core/graphics/gles3/storage/gl_mesh.h"
#include "core/graphics/gles3/storage/gl_texture_2d.h"
#include "core/graphics/gles3/storage/gl_texture_cubemap.h"
#include "core/graphics/rendering_device.h"


namespace golias {


    class RenderingDeviceGLES3 final : public RenderingDevice {
    public:
        RenderingDeviceGLES3();

        ~RenderingDeviceGLES3();

        bool Initialize(SDL_Window* sdl_window) override;

        void BindShader(Shader* shader) override;
        void BindMesh(Mesh* mesh) override;
        void UnbindMesh(Mesh* mesh) override;
        void BindTexture(Shader* shader, std::string_view uniformName, Uint32 slot, Texture2D* texture) override;
        void BindCubemap(Shader* shader, std::string_view uniformName, Uint32 slot, TextureCubemap* texture) override;
        void BindMaterial(Material* material) override;

        std::shared_ptr<Shader> GetDefaultShader3D() const override;
        std::shared_ptr<Shader> GetDefaultShader2D() const override;
        std::shared_ptr<Shader> GetDefaultShaderCanvas() const override;
        std::shared_ptr<Shader> GetDefaultShadowMapShader() const override;
        std::shared_ptr<Shader> GetDefaultSkyboxShader() const override;

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

        void ClearColor(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) override;
        void ClearBuffer(EClearFlags flags) override;

        void DrawMesh(Mesh* mesh) override;

        void SwapChain() override;

        void SetViewport(const Viewport& vp) override;
        void SetScissor(const Scissor& scissor) override;


        std::shared_ptr<Framebuffer> GetDefaultFramebuffer() override;
        std::shared_ptr<Framebuffer> GetDefaultShadowMapFramebuffer() override;

        std::shared_ptr<TextureCubemap> CreateCubemapFromFaces(const std::array<std::string, 6>& faces) override;
        std::shared_ptr<TextureCubemap> CreateCubemapFromCross(const std::string& crossPath) override;
        std::shared_ptr<TextureCubemap> CreateCubemapProcedural() override;

        std::shared_ptr<Texture2D> GetWhiteTexture2D() const override;
        std::shared_ptr<Texture2D> GetNormalTexture2D() const override;
        std::shared_ptr<TextureCubemap> GetWhiteTextureCubemap() const override;

        void SetColorWrite(bool red, bool green, bool blue, bool alpha) override;
        void SetBlendMode(EBlendMode blendMode) override;
        void SetDepthComparison(EComparisonFunc func) override;
        void SetCullMode(ECullMode cullMode) override;
        void SetDepthWrite(bool enable) override;
        void SetDepthTest(bool enable) override;

    private:
        SDL_GLContext gl_context = nullptr;

        std::shared_ptr<Shader> default_shader_3d     = nullptr;
        std::shared_ptr<Shader> default_shader_2d     = nullptr;
        std::shared_ptr<Shader> default_shader_canvas = nullptr;
        std::shared_ptr<Shader> default_shader_csm    = nullptr;
        std::shared_ptr<Shader> skybox_shader         = nullptr;


        std::shared_ptr<Framebuffer> shadowFBO = nullptr;

        bool CreateDefaultShaders();
        bool CreateDefaultFramebuffers();
        bool CreateSkyboxShader();
        bool CreateDefaultTextures();
    };
}; // namespace golias
