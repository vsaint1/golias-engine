#pragma once
#include "core/io/file_system.h"


/**
 * @brief UberShader shader effects for rendering texts and sprites.
 */
struct UberShader {
    bool use_outline = false;
    bool use_shadow = false;
    glm::vec4 outline_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 shadow_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
    float outline_width = 1.0f;
    glm::vec2 shadow_offset = glm::vec2(2.0f, 2.0f);

    static UberShader none() {
        return UberShader{};
    }

    bool is_none() const  {
        return !use_outline && !use_shadow;
    }

    static UberShader outline_only(const glm::vec4& color = glm::vec4(0,0,0,1), float width = 1.0f) {
        UberShader effects;
        effects.use_outline = true;
        effects.outline_color = color;
        effects.outline_width = width;
        return effects;
    }

    static UberShader shadow_only(const glm::vec4& color = glm::vec4(0,0,0,0.5f),
                                    const glm::vec2& offset = glm::vec2(2,2)) {
        UberShader effects;
        effects.use_shadow = true;
        effects.shadow_color = color;
        effects.shadow_offset = offset;
        return effects;
    }

    static UberShader both(const glm::vec4& outline_col = glm::vec4(0,0,0,1),
                             const glm::vec4& shadow_col = glm::vec4(0,0,0,0.5f),
                             float width = 1.0f, const glm::vec2& offset = glm::vec2(2,2)) {
        UberShader effects;
        effects.use_outline = true;
        effects.use_shadow = true;
        effects.outline_color = outline_col;
        effects.shadow_color = shadow_col;
        effects.outline_width = width;
        effects.shadow_offset = offset;
        return effects;
    }

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
