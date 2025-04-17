#pragma once
#include "core/renderer/shader.h"



/*!

    @brief Opengl Shader implementation
    - GLSL shader header (API `OpenGL`)
    - Compile the shader
    - Use the shader
    - Send uniforms

    @version 0.0.9
    @param string vertex The shader source
    @param string fragment The shader source
*/

class OpenglShader : public Shader {
public:
    OpenglShader() = default;

    template <typename T>
    T GetValue(const std::string& name);

    OpenglShader(const std::string& vertex, const std::string& fragment);

    void Bind() const;

    void SetValue(const std::string& name, float value) const;

    void SetValue(const std::string& name, int value) const;

    void SetValue(const std::string& name, glm::mat4 value) const;

    void SetValue(const std::string& name, glm::vec2 value) const;

    void SetValue(const std::string& name, glm::vec3 value) const;

    void SetValue(const std::string& name, glm::vec4 value) const;

    void Destroy();

    unsigned int GetID() const;

    bool IsValid() const;

private:
    unsigned int GetUniformLocation(const std::string& name) const;

    unsigned int CompileShader(unsigned int type, const char* source);
};

// Default shader only
struct ShaderEffect {
    struct Shadow_t {
        bool enabled = false;
        glm::vec4 color{};
        glm::vec2 offset{};
    };

    struct Outline_t {
        bool enabled = false;
        glm::vec4 color{};
        float thickness = 1.0f;
    };

    struct Glow_t {
        bool enabled = false;
        glm::vec4 color{};
        float strength = 1.0f;
        float radius   = 1.0f;
    };
};

template <typename T>
inline T OpenglShader::GetValue(const std::string& name) {
    unsigned int location = GetUniformLocation(name);
    if (location == -1) {
        LOG_ERROR("Shader variable not found: %s", name.c_str());
        return T();
    }

    if constexpr (std::is_same_v<T, float>) {
        float value;
        glGetUniformfv(id, location, &value);
        return value;
    } else if constexpr (std::is_same_v<T, int>) {
        int value;
        glGetUniformiv(id, location, &value);
        return value;
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
        GLfloat data[2];
        glGetUniformfv(id, location, data);
        return glm::vec2(data[0], data[1]);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
        GLfloat data[3];
        glGetUniformfv(id, location, data);
        return glm::vec3(data[0], data[1], data[2]);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
        GLfloat data[4];
        glGetUniformfv(id, location, data);
        return glm::vec4(data[0], data[1], data[2], data[3]);
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
        GLfloat data[16];
        glGetUniformfv(id, location, data);
        return glm::make_mat4(data);
    } else {
        LOG_ERROR("Unsupported type: %s", typeid(T).name());
    }

    return T();
}
