#include "graphics/graphics_device.h"

#include "graphics/shader.h"
#include "render/material.h"
#include <glad.h>
#include <glfw/glfw3.h>

namespace golias {

    GraphicsDevice::~GraphicsDevice() = default;

    bool GraphicsDevice::Initialize() {

#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)


        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return false;
        }
#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

        if (!gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return false;
        }

#endif

        glEnable(GL_DEPTH_TEST);

        const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

        GOLIAS_LOG_INFO("OpenGL Vendor: %s | Device: %s", vendor, renderer);
        GOLIAS_LOG_INFO("OpenGL Version: %s", version);

        return true;
    }

    Ref<Shader> GraphicsDevice::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {

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

        return std::make_shared<Shader>(program);
    }

    GLuint GraphicsDevice::CreateVertexBuffer(const std::vector<float>& vertices) {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        return VBO;
    }


    GLuint GraphicsDevice::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        return EBO;
    }

    void GraphicsDevice::SetClearColor(const Color& color) {
        glClearColor(color.R, color.G, color.B, color.A);
    }

    void GraphicsDevice::ClearBuffers(GLbitfield mask) {
        glClear(mask);
    }

    void GraphicsDevice::SetViewport(const Viewport& viewport) {
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

    GLuint GraphicsDevice::CreateUniformBuffer(size_t size) {
        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return buffer;
    }

    void GraphicsDevice::UpdateUniformBuffer(GLuint buffer, const void* data, size_t size, size_t offset) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
