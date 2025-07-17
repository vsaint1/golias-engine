#pragma once
#include "core/io/file_system.h"


class ShaderEffect {
public:
    struct {

        int bEnabled    = 0;
        glm::vec4 color = glm::vec4(1.0f);
        float thickness = 0.0f;

    } Outline;

    struct {

        int bEnabled           = 0;
        glm::vec4 color        = glm::vec4(1.0f);
        glm::vec2 pixel_offset = glm::vec2(0.0f);

    } Shadow;

    struct {
        int bEnabled = 0;
    } Glow;
};


/*!

    @brief Shader Abstract class
    - Compile the shader
    - Bind the shader
    - Send uniforms

    @version  0.0.2
    @param string vertex The shader source
    @param string fragment The shader source
*/
class Shader {
public:
    Shader() = default;

    virtual ~Shader() = default;

    virtual void bind() const = 0;

    virtual void set_value(const std::string& name, float value) = 0;

    virtual void set_value(const std::string& name, int value) = 0;

    virtual void set_value(const std::string& name, const int* values, Uint32 count) = 0;

    virtual void set_value(const std::string& name, const float* values, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::mat4 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec2 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec3 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, glm::vec4 value, Uint32 count) = 0;

    virtual void set_value(const std::string& name, unsigned int value) = 0;


    virtual void destroy() = 0;

    virtual unsigned int get_id() const = 0;

    virtual bool is_valid() const = 0;


protected:
    unsigned int id = 0;

    std::unordered_map<std::string, unsigned int> _uniforms;
};
