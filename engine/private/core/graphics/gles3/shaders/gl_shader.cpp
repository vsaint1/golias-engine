#include "core/graphics/gles3/shaders/gl_shader.h"


namespace golias {

    OpenglShader::OpenglShader(const std::string& vertex, const std::string& fragment) {
        handle = CreateProgram(vertex, fragment);
    }

    OpenglShader::~OpenglShader() {
        glDeleteProgram(handle);
    }

    void OpenglShader::Bind() {
        glUseProgram(handle);
        current_texture_unit = 0;
    }

    void OpenglShader::Unbind() {
        glUseProgram(0);
    }

    int32_t OpenglShader::GetUniformLocation(const std::string_view pName) {
        std::string name_str(pName);

        auto it = uniform_location_cache.find(name_str);

        if (it != uniform_location_cache.end()) {
            return it->second;
        }

        GLint location = glGetUniformLocation(handle, name_str.c_str());

        if (location != -1) {
            uniform_location_cache[name_str] = static_cast<int32_t>(location);
            return static_cast<int32_t>(location);
        }

        return -1;
    }

    void OpenglShader::SetUniform(const std::string_view pName, const UniformValue& value) {

        GLint location = GetUniformLocation(pName);

        if (location == -1) {
            // spdlog::warn("OpenglShader::SetUniform: Uniform '{}' not found in shader.", pName);
            return;
        }


        std::visit(
            [&](auto&& v) {
                using T = std::decay_t<decltype(v)>;

                if constexpr (std::is_same_v<T, int>) {
                    glUniform1i(location, v);
                }else if constexpr (std::is_same_v<T, bool>) {
                    glUniform1i(location, v ? 1 : 0);
                } else if constexpr (std::is_same_v<T, float>) {
                    glUniform1f(location, v);
                } else if constexpr (std::is_same_v<T, glm::vec2>) {
                    glUniform2fv(location, 1, &v.x);
                } else if constexpr (std::is_same_v<T, glm::vec3>) {
                    glUniform3fv(location, 1, &v.x);
                } else if constexpr (std::is_same_v<T, glm::vec4>) {
                    glUniform4fv(location, 1, &v.x);
                } else if constexpr (std::is_same_v<T, glm::mat4>) {
                    glUniformMatrix4fv(location, 1, GL_FALSE, &v[0][0]);
                } else if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
                    if (v && v.get()) {
                        glActiveTexture(GL_TEXTURE0 + current_texture_unit);
                        glBindTexture(GL_TEXTURE_2D, v->GetNativeHandle());
                        glUniform1i(location, current_texture_unit);
                        ++current_texture_unit;
                    }
                }
            },
            value);
    }

    GLuint OpenglShader::CompileShader(GLenum type, const std::string& source) {
        GLuint shader   = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            spdlog::error("OpenglShader::CompileShader Failed, Reason {} - Type {}", infoLog, GetShaderTypeCStr(type));
            glDeleteShader(shader);
            return 0;
        }

        spdlog::info("OpenglShader::CompileShader Succeeded, Shader Handle {}, Type {}", shader, GetShaderTypeCStr(type));
        return shader;
    }

    GLuint OpenglShader::CreateProgram(const std::string& vertexSource, const std::string& fragmentSource) {

        GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        GLuint program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            spdlog::error("OpenglShader::CreateProgram Failed, Reason {}", infoLog);
            glDeleteProgram(program);
            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        spdlog::info("OpenglShader::CreateProgram Succeeded, Shader Handle {}", program);
        return program;
    }

    const char* OpenglShader::GetShaderTypeCStr(GLenum type) {
        switch (type) {
        case GL_VERTEX_SHADER:
            return "VERTEX_SHADER";
        case GL_FRAGMENT_SHADER:
            return "FRAGMENT_SHADER";
        case GL_COMPUTE_SHADER:
            return "COMPUTE_SHADER";
        default:
            return "UNKNOWN_SHADER_TYPE";
        }
    }

} // namespace golias
