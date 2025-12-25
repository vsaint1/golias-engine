#include "core/graphics/gles3/rendering_device_gles3.h"

#include "core/graphics/rendering_device.h"
#include "core/graphics/gles3/gl_common.h"

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


        spdlog::info("RenderingDeviceGLES3::Initialize Initialized successfully GLES3 Rendering Device.");

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
        }
    }

    void RenderingDeviceGLES3::DrawMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Draw();
        }
    }

    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromSource(const std::string& vertexSource,
                                                                         const std::string& fragmentSource) {

        return std::make_shared<OpenglShader>(vertexSource, fragmentSource);
    }

    std::shared_ptr<Mesh> RenderingDeviceGLES3::CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices,
                                                                   const std::vector<uint32_t>& indices) {
        return std::make_shared<OpenglMesh>(layout, vertices, indices);
    }

    void RenderingDeviceGLES3::Clear(glm::vec4 color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        clear_color = color;
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


    std::shared_ptr<Shader> RenderingDeviceGLES3::CreateShaderFromFile(const std::string_view pFilePath) {

        spdlog::warn("RenderingDeviceGLES3::CreateShaderFromFile not implemented yet.");

        return nullptr;
    }

    void RenderingDeviceGLES3::Present() {
        SDL_GL_SwapWindow(window);
    }

    RenderingDeviceGLES3::~RenderingDeviceGLES3() {
        if (gl_context) {
            SDL_GL_DestroyContext(gl_context);
            gl_context = nullptr;
        }

        spdlog::info("RenderingDeviceGLES3::~RenderingDeviceGLES3 GLES3 Rendering Device destroyed.");
    }


}; // namespace golias
