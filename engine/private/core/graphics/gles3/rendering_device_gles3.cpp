#include "core/graphics/gles3/rendering_device_gles3.h"

#include "core/engine.h"
#include "core/graphics/gles3/gl_common.h"
#include "core/graphics/gles3/gl_physics_debug_drawer.h"
#include "core/graphics/gles3/storage/gl_framebuffer.h"
#include "core/graphics/rendering_device.h"

namespace golias {


    bool RenderingDeviceGLES3::Initialize(SDL_Window* sdl_window) {
        window = sdl_window;

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    #if defined(SDL_PLATFORM_MACOS)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // OSX compatibility Bit
    #endif


#endif

        gl_context = SDL_GL_CreateContext(sdl_window);

        if (!gl_context) {
            spdlog::critical("RenderingDeviceGLES3::Initialize Failed to create OpenGL/ES context: {}", SDL_GetError());
            return false;
        }


#if defined(SDL_PLATFORM_ANDROID) && !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_EMSCRIPTEN)
        if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to initialize OpenGL Loader (GLAD)");
            return false;
        }
#else
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to initialize OpenGLES Loader (GLAD)");
            return false;
        }
#endif

        if (!CreateDefaultShaders()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default shaders.");
            return false;
        }

        if (!CreateDefaultFramebuffers()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default framebuffers.");
            return false;
        }

        if (!CreateSkyboxShader()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create skybox shader.");
            return false;
        }

        if (!CreateDefaultTextures()) {
            spdlog::error("RenderingDeviceGLES3::Initialize Failed to create default textures.");
            return false;
        }

        spdlog::info("RenderingDeviceGLES3::Initialize Initialized successfully GLES3 Rendering Device.");

        physicsDebug3D = std::make_unique<PhysicsDebugDrawerGLES3>();

        glEnable(GL_DEPTH_TEST);
        return true;
    }

    void RenderingDeviceGLES3::BindShader(Shader* shader) {
        if (shader) {
            shader->Bind();
        }
    }


    void RenderingDeviceGLES3::BindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Bind();
        }
    }

    void RenderingDeviceGLES3::BindMaterial(Material* material) {
        if (material) {
            material->Activate();

            if (material->IsDepthTestEnabled()) {
                SetDepthTest(true);
                SetDepthComparison(material->GetDepthFunc());
            } else {
                SetDepthTest(false);
            }

            SetDepthWrite(material->IsDepthWriteEnabled());

            SetBlendMode(material->GetBlendMode());

            SetCullMode(material->GetCullMode());
        }
    }

    void RenderingDeviceGLES3::DrawMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Draw();
        }
    }

    void RenderingDeviceGLES3::UnbindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Unbind();
        }
    }


    void RenderingDeviceGLES3::BindTexture(Shader* shader, std::string_view uniformName, Uint32 slot, Texture2D* texture) {
        if (!texture) {
            return;
        }

        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture->GetNativeHandle());

        GLint loc = shader->GetUniformLocation(uniformName);
        if (loc != -1) {
            glUniform1i(loc, slot);
        }
    }

    void RenderingDeviceGLES3::BindCubemap(Shader* shader, std::string_view uniformName, Uint32 slot, TextureCubemap* texture) {
        if (!texture) {
            return;
        }

        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->GetNativeHandle());

        GLint loc = shader->GetUniformLocation(uniformName);
        if (loc != -1) {
            glUniform1i(loc, slot);
        }
    }

    PhysicsDebugDrawer* RenderingDeviceGLES3::GetPhysicsDebugDrawer() {
        return physicsDebug3D.get();
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromFile(const std::string_view pFilePath) {

        auto& fileSystem = golias::Engine::GetInstance().GetFileSystem();

        return std::make_shared<OpenglTexture2D>(fileSystem.GetAssetFile(pFilePath));
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) {

        return std::make_shared<OpenglTexture2D>(w, h, format, const_cast<Uint8*>(data));
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromSource(const std::string& vertexSource,
                                                                         const std::string& fragmentSource) {

        return std::make_shared<OpenglShader>(vertexSource, fragmentSource);
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::GetWhiteTexture2D() const {
        return whiteTexture2D;
    }

    std::shared_ptr<Texture2D> RenderingDeviceGLES3::GetNormalTexture2D() const {
        return normalTexture2D;
    }


    std::shared_ptr<TextureCubemap> RenderingDeviceGLES3::GetWhiteTextureCubemap() const {
        
        return whiteTextureCubemap;
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMeshFromData(const VertexLayout& layout,
                                                                   const std::vector<float>& vertices,

                                                                   const std::vector<uint32_t>& indices) {

        if (indices.empty()) {
            return std::make_shared<OpenglMesh>(layout, vertices);
        } else {
            return std::make_shared<OpenglMesh>(layout, vertices, indices);
        }
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMesh() {
        return std::shared_ptr<OpenglMesh>();
    }


    void RenderingDeviceGLES3::ClearColor(glm::vec4 color) {
        glClearColor(color.r, color.g, color.b, color.a);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        clear_color = color;
    }


    void RenderingDeviceGLES3::ClearBuffer(EClearFlags flags) {
        GLbitfield glFlags = ToGLClearFlags(flags);

        glClear(glFlags);
    }

    Buffer RenderingDeviceGLES3::CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) {
        Buffer buffer;
        glGenBuffers(1, &buffer.handle);

        GLint gl_buffer_target = ToGLBuferTarget(bufferTarget);
        glBindBuffer(gl_buffer_target, buffer.handle);
        glBufferData(gl_buffer_target, size, data, ToGLBufferUsage(bufferFlags));
        glBindBuffer(gl_buffer_target, 0);

        buffer.size        = size;
        buffer.usage_flags = bufferFlags;
        buffer.target      = bufferTarget;
        return buffer;
    }


    std::shared_ptr<Framebuffer> RenderingDeviceGLES3::CreateFramebuffer(const FramebufferSpec& speficiation) {
        return std::make_shared<OpenglFramebuffer>(speficiation);
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromFile(const std::string_view pFilePath) {

        spdlog::warn("RenderingDeviceGLES3::CreateShaderFromFile not implemented yet.");

        return nullptr;
    }

    void RenderingDeviceGLES3::SwapChain() {
        SDL_GL_SwapWindow(window);
    }

    bool RenderingDeviceGLES3::CreateDefaultShaders() {
        const auto& fileSystem = Engine::GetInstance().GetFileSystem();

        std::string vertexSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_3d.vert");

        std::string fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_3d.frag");

        default_shader_3d = std::make_shared<OpenglShader>(vertexSource, fragmentSource);


        if (!default_shader_3d) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default 3D shader.");
            return false;
        }

        vertexSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_2d.vert");

        fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_2d.frag");

        default_shader_2d = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_2d) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default 2D shader.");
            return false;
        }

        vertexSource   = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_canvas.vert");
        fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/default_canvas.frag");

        default_shader_canvas = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_canvas) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create default canvas shader.");
            return false;
        }

        vertexSource   = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/shadow_map.vert");
        fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/shadow_map.frag");

        default_shader_csm = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!default_shader_csm) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create shadow map shader.");
            return false;
        }

        vertexSource   = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/skybox.vert");
        fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/skybox.frag");

        skybox_shader = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!skybox_shader) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultShaders Failed to create skybox shader.");
            return false;
        }


        return true;
    }

    void RenderingDeviceGLES3::SetViewport(const Viewport& vp) {
        viewport = vp;
        glViewport(vp.x, vp.y, vp.width, vp.height);
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShader3D() const {
        return default_shader_3d;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShader2D() const {
        return default_shader_2d;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShaderCanvas() const {
        return default_shader_canvas;
    }


    bool RenderingDeviceGLES3::CreateDefaultFramebuffers() {
        FramebufferSpec shadowFboSpec;
        shadowFboSpec.width       = 4096;
        shadowFboSpec.height      = 4096;
        shadowFboSpec.attachments = {
            FramebufferAttachmentSpec(EFramebufferAttachment::DEPTH_ATTACHMENT, ETextureFormat::DEPTH32F)};

        shadowFBO = std::make_shared<OpenglFramebuffer>(shadowFboSpec);

        if (!shadowFBO) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultFramebuffers Failed to create shadow map Framebuffer");
            return false;
        }

        spdlog::info("RenderingDeviceGLES3::CreateDefaultFramebuffers Created shadow map Framebuffer");

        return true;
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultShadowMapShader() const {
        return default_shader_csm;
    }


    bool RenderingDeviceGLES3::CreateSkyboxShader() {
        const auto& fileSystem = Engine::GetInstance().GetFileSystem();

        std::string vertexSource   = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/skybox.vert");
        std::string fragmentSource = GetShaderHeaderVersion() + fileSystem.LoadAssetFileText("shaders/skybox.frag");

        skybox_shader = std::make_shared<OpenglShader>(vertexSource, fragmentSource);

        if (!skybox_shader) {
            spdlog::error("RenderingDeviceGLES3::CreateSkyboxShader Failed to create skybox shader.");
            return false;
        }

        return true;
    }


    bool RenderingDeviceGLES3::CreateDefaultTextures() {
        Uint8* whitePixel = new Uint8[4]{255, 255, 255, 255};
        whiteTexture2D    = std::make_shared<OpenglTexture2D>(1, 1, ETextureFormat::RGBA8, whitePixel);

        if (!whiteTexture2D) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultTextures Failed to create white texture.");
            return false;
        }

        Uint8* normalPixel = new Uint8[4]{128, 128, 255, 255};
        normalTexture2D    = std::make_shared<OpenglTexture2D>(1, 1, ETextureFormat::RGBA8, normalPixel);
        if (!normalTexture2D) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultTextures Failed to create normal texture.");
            return false;
        }

        Uint8* whiteCubemapPixel = new Uint8[6 * 4]{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

        whiteTextureCubemap = std::make_shared<OpenglTextureCubemap>(1, 1, ETextureFormat::RGBA8, whiteCubemapPixel);
       
        if (!whiteTextureCubemap) {
            spdlog::error("RenderingDeviceGLES3::CreateDefaultTextures Failed to create white cubemap texture.");
            return false;
        }

        return true;
    }

    std::shared_ptr<TextureCubemap> RenderingDeviceGLES3::CreateCubemapFromFaces(const std::array<std::string, 6>& faces) {

        return std::make_shared<OpenglTextureCubemap>(faces);
    }


    std::shared_ptr<TextureCubemap> RenderingDeviceGLES3::CreateCubemapFromCross(const std::string& crossPath) {
        const auto& fileSystem = Engine::GetInstance().GetFileSystem();


        return std::make_shared<OpenglTextureCubemap>(fileSystem.GetAssetFile(crossPath));
    }


    std::shared_ptr<TextureCubemap> RenderingDeviceGLES3::CreateCubemapProcedural() {
        return std::make_shared<OpenglTextureCubemap>();
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::GetDefaultSkyboxShader() const {
        return skybox_shader;
    }


    RenderingDeviceGLES3::RenderingDeviceGLES3() {
    }

    std::shared_ptr<Framebuffer> RenderingDeviceGLES3::GetDefaultFramebuffer() {
        return nullptr;
    }

    std::shared_ptr<Framebuffer> RenderingDeviceGLES3::GetDefaultShadowMapFramebuffer() {
        return shadowFBO;
    }


    void RenderingDeviceGLES3::SetScissor(const Scissor& scissor) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
    }

    void RenderingDeviceGLES3::SetColorWrite(bool red, bool green, bool blue, bool alpha) {
        glColorMask(red ? GL_TRUE : GL_FALSE, green ? GL_TRUE : GL_FALSE, blue ? GL_TRUE : GL_FALSE, alpha ? GL_TRUE : GL_FALSE);
    }
    void RenderingDeviceGLES3::SetBlendMode(EBlendMode blendMode) {


        switch (blendMode) {
        case EBlendMode::BLEND_MODE_DISABLED:
            glDisable(GL_BLEND);
            break;
        case EBlendMode::BLEND_MODE_OPAQUE:
            glDisable(GL_BLEND);
            break;
        case EBlendMode::BLEND_MODE_ALPHA:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case EBlendMode::BLEND_MODE_ADDITIVE:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case EBlendMode::BLEND_MODE_MULTIPLY:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
        default:
            break;
        }
    }

    void RenderingDeviceGLES3::SetDepthComparison(EComparisonFunc func) {
        switch (func) {
        case EComparisonFunc::COMPARISON_NEVER:
            glDepthFunc(GL_NEVER);
            break;
        case EComparisonFunc::COMPARISON_LESS:
            glDepthFunc(GL_LESS);
            break;
        case EComparisonFunc::COMPARISON_EQUAL:
            glDepthFunc(GL_EQUAL);
            break;
        case EComparisonFunc::COMPARISON_LESS_EQUAL:
            glDepthFunc(GL_LEQUAL);
            break;
        case EComparisonFunc::COMPARISON_GREATER:
            glDepthFunc(GL_GREATER);
            break;
        case EComparisonFunc::COMPARISON_NOT_EQUAL:
            glDepthFunc(GL_NOTEQUAL);
            break;
        case EComparisonFunc::COMPARISON_GREATER_EQUAL:
            glDepthFunc(GL_GEQUAL);
            break;
        case EComparisonFunc::COMPARISON_ALWAYS:
            glDepthFunc(GL_ALWAYS);
            break;
        default:
            glDepthFunc(GL_LESS);
            break;
        }
    }

    void RenderingDeviceGLES3::SetCullMode(ECullMode cullMode) {

        switch (cullMode) {
        case ECullMode::CULL_MODE_DISABLED:
            glDisable(GL_CULL_FACE);
            break;
        case ECullMode::CULL_MODE_FRONT:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        case ECullMode::CULL_MODE_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        case ECullMode::CULL_MODE_FRONT_AND_BACK:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
        }
    }

    void RenderingDeviceGLES3::SetDepthWrite(bool enable) {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }

    void RenderingDeviceGLES3::SetDepthTest(bool enable) {
        if (enable) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    RenderingDeviceGLES3::~RenderingDeviceGLES3() {

        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
            gl_context = nullptr;
        }
    }


}; // namespace golias
