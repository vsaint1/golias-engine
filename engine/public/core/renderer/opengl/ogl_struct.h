#pragma once
#include "core/renderer/base_struct.h"


class OpenglShader final : public Shader {
public:
    OpenglShader() = default;
    ~OpenglShader();

    template <typename T>
    T get_value(const std::string& name);

    OpenglShader(const std::string& vertex, const std::string& fragment);

    void bind() const override;

    void set_value(const std::string& name, float value) override;

    void set_value(const std::string& name, int value) override;

    void set_value(const std::string& name, Uint32 value) override;

    void set_value(const std::string& name, glm::mat4 value, Uint32 count = 1) override;

    void set_value(const std::string& name, const int* value, Uint32 count = 1) override;

    void set_value(const std::string& name, const float* value, Uint32 count = 1) override;

    void set_value(const std::string& name, glm::vec2 value, Uint32 count = 1) override;

    void set_value(const std::string& name, glm::vec3 value, Uint32 count = 1) override;

    void set_value(const std::string& name, glm::vec4 value, Uint32 count = 1) override;

    void destroy() override;

    unsigned int get_id() const override;

    bool is_valid() const override;

private:
    Uint32 get_uniform_location(const std::string& name);

    Uint32 compile_shader(Uint32 type, const char* source);
};


template <typename T>
inline T OpenglShader::get_value(const std::string& name) {
    unsigned int location = get_uniform_location(name);
    if (location == -1) {
        printf("Shader variable not found: %s\n", name.c_str());
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
        printf("Unsupported type: %s\n", typeid(T).name());
    }

    return T();
}
