#include "graphics/graphics_device.h"

#include "graphics/shader.h"
#include "render/material.h"

namespace golias {

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
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void GraphicsDevice::ClearBuffers(GLbitfield mask) {
        glClear(mask);
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
} // namespace golias
