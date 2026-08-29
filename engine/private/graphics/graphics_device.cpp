#include "graphics/graphics_device.h"

#include "graphics/framebuffer.h"
#include "graphics/shader.h"
#include "graphics/texture_2d_array.h"
#include "graphics/texture_cube.h"
#include "render/material.h"
#include "render/mesh.h"


namespace golias {

    GraphicsDevice::~GraphicsDevice() = default;

    Ref<Texture2DArray> GraphicsDevice::CreateTexture2DArray(const TextureDesc& desc) {
        return std::make_shared<Texture2DArray>(desc);
    }

    Ref<Framebuffer> GraphicsDevice::CreateFramebuffer(const TextureDesc& desc) {
        return std::make_shared<Framebuffer>(desc);
    }

    Ref<TextureCube> GraphicsDevice::CreateTextureCube(const TextureDesc& desc) {
        return std::make_shared<TextureCube>(desc);
    }

    bool GraphicsDevice::Initialize() {


        glEnable(GL_DEPTH_TEST);

        const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

        GOLIAS_LOG_INFO("OpenGL Vendor: %s | Device: %s", vendor, renderer);
        GOLIAS_LOG_INFO("OpenGL Version: %s", version);

        return true;
    }

    Ref<Shader> GraphicsDevice::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
        std::string cacheKey = vertexSource;
        cacheKey.push_back('\0');
        cacheKey += fragmentSource;

        if (const auto it = mShaderCache.find(cacheKey); it != mShaderCache.end()) {
            return it->second;
        }

        GLuint vertexShader          = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSourceCStr = vertexSource.c_str();

        glShaderSource(vertexShader, 1, &vertexSourceCStr, nullptr);
        glCompileShader(vertexShader);


        GLint vertexCompileStatus;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexCompileStatus);

        if (vertexCompileStatus != GL_TRUE) {
            GLint logLength;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetShaderInfoLog(vertexShader, logLength, nullptr, log.data());
            GOLIAS_LOG_ERROR("Vertex shader compilation failed: %s", log.data());
            glDeleteShader(vertexShader);
            return nullptr;
        }


        GLuint fragmentShader          = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSourceCStr = fragmentSource.c_str();

        glShaderSource(fragmentShader, 1, &fragmentSourceCStr, nullptr);
        glCompileShader(fragmentShader);

        GLint fragmentCompileStatus;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompileStatus);

        if (fragmentCompileStatus != GL_TRUE) {
            GLint logLength;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetShaderInfoLog(fragmentShader, logLength, nullptr, log.data());
            GOLIAS_LOG_ERROR("Fragment shader compilation failed: %s", log.data());
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return nullptr;
        }

        GOLIAS_LOG_INFO("Vertex and fragment shaders compiled successfully.");

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);

        glAttachShader(program, fragmentShader);

        glLinkProgram(program);

        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

        if (linkStatus != GL_TRUE) {
            GLint logLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
            GOLIAS_LOG_ERROR("Shader program linking failed: %s", log.data());
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(program);
            return nullptr;
        }

        GOLIAS_LOG_INFO("Shader program linked successfully with ID: %u", program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        GOLIAS_LOG_INFO("Shader program created successfully with ID: %u", program);

        Ref<Shader> shader = std::make_shared<Shader>(program);
        mShaderCache.emplace(std::move(cacheKey), shader);
        return shader;
    }

    void GraphicsDevice::SetClearColor(const Color& color) {
        glClearColor(color.R, color.G, color.B, color.A);
    }

    void GraphicsDevice::SetCullMode(CullMode mode) {
        if (mode == CullMode::None) {
            glDisable(GL_CULL_FACE);
            return;
        }

        glEnable(GL_CULL_FACE);

        GLenum faceMode;
        switch (mode) {
        case CullMode::Front:
            faceMode = GL_FRONT;
            break;
        case CullMode::Back:
            faceMode = GL_BACK;
            break;
        case CullMode::FrontAndBack:
            faceMode = GL_FRONT_AND_BACK;
            break;
        default:
            faceMode = GL_BACK;
            break;
        }

        glCullFace(faceMode);
    }

    void GraphicsDevice::ClearBuffers(ClearFlag flag) {
        GLbitfield mask = 0;

        if (HasFlag(flag, ClearFlag::Color)) {
            mask |= GL_COLOR_BUFFER_BIT;
        }

        if (HasFlag(flag, ClearFlag::Depth)) {
            mask |= GL_DEPTH_BUFFER_BIT;
        }

        if (HasFlag(flag, ClearFlag::Stencil)) {
            mask |= GL_STENCIL_BUFFER_BIT;
        }

        glClear(mask);
    }

    void GraphicsDevice::SetDepthTestEnabled(bool enabled) {
        if (enabled) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    uint32_t GraphicsDevice::CreateBuffer(const BufferDesc& desc) {
        GLuint buffer;
        glGenBuffers(1, &buffer);

        GLenum target         = GL_ARRAY_BUFFER;
        const char* targetStr = "VERTEX_BUFFER";

        if (HasFlag(desc.Target, BufferTarget::Vertex)) {
            target = GL_ARRAY_BUFFER;
        } else if (HasFlag(desc.Target, BufferTarget::Index)) {
            target    = GL_ELEMENT_ARRAY_BUFFER;
            targetStr = "INDEX_BUFFER";
        } else if (HasFlag(desc.Target, BufferTarget::Uniform)) {
            target    = GL_UNIFORM_BUFFER;
            targetStr = "UNIFORM_BUFFER";
        }


        GLenum usage         = GL_STATIC_DRAW;
        const char* usageStr = "Static";
        if (desc.Usage == BufferUsage::Dynamic) {
            usage    = GL_DYNAMIC_DRAW;
            usageStr = "Dynamic";
        } else if (desc.Usage == BufferUsage::Stream) {
            usage    = GL_STREAM_DRAW;
            usageStr = "Streaming";
        }

        glBindBuffer(target, buffer);
        glBufferData(target, desc.Size, nullptr, usage);
        glBindBuffer(target, 0);

        GOLIAS_LOG_TRACE("Target %s | Usage %s | Buffer Handle %#u | Size %zu", targetStr, usageStr, buffer, desc.Size);

        return buffer;
    }

    void GraphicsDevice::SetBlendMode(BlendMode mode) {

        switch (mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
            break;
        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::Multiply:
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            break;
        }
    }

    void GraphicsDevice::UpdateBuffer(uint32_t handle, BufferTarget target, const void* data, const size_t size, const size_t offset) {

        GLenum internalTarget = GL_ARRAY_BUFFER;

        if (HasFlag(target, BufferTarget::Vertex)) {
            internalTarget = GL_ARRAY_BUFFER;
        } else if (HasFlag(target, BufferTarget::Index)) {
            internalTarget = GL_ELEMENT_ARRAY_BUFFER;
        } else if (HasFlag(target, BufferTarget::Uniform)) {
            internalTarget = GL_UNIFORM_BUFFER;
        }

        glBindBuffer(internalTarget, handle);
        glBufferSubData(internalTarget, offset, size, data);
        glBindBuffer(internalTarget, 0);
    }


    void GraphicsDevice::SetDepthWriteEnabled(bool enabled) {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    }

    const Viewport& GraphicsDevice::GetViewport() const {
        return mViewport;
    }

    void GraphicsDevice::SetViewport(const Viewport& viewport) {
        mViewport = viewport;
        glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
    }

    void GraphicsDevice::BindShader(Shader* shader) {
        if (shader) {
            shader->Bind();
        }
    }

    void GraphicsDevice::BindMaterial(Material* material) {
        if (material) {
            material->Bind();
        }
    }


    void GraphicsDevice::BindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Bind();
        }
    }

    void GraphicsDevice::DrawMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Draw();
        }
    }

    void GraphicsDevice::UnbindMesh(Mesh* mesh) {
        if (mesh) {
            mesh->Unbind();
        }
    }


    void GraphicsDevice::BindUniformBuffer(GLuint buffer, uint32_t binding) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer);
    }

    void GraphicsDevice::DestroyBuffer(GLuint buffer) {
        if (buffer) {
            glDeleteBuffers(1, &buffer);
        }
    }
} // namespace golias
