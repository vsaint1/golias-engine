#pragma once
#include "core/gstl/str.h"
#include "servers/rendering/rendering_device.h"

/// Variant type for shader parameter values
using UniformType = std::variant<float, glm::vec2, glm::vec3, glm::vec4, int>;

class CanvasMaterial {
public:
    CanvasMaterial() = default;

    explicit CanvasMaterial(const RID shader_rid) : shader(shader_rid) {
    }

    CanvasMaterial& set_shader(RID shader_rid);

    CanvasMaterial& set_color(const Color& col);

    CanvasMaterial& set_texture(RID tex);

    CanvasMaterial& set_custom_texture(const char* name, RID tex);

    template<typename T>
    CanvasMaterial& set_shader_param(const char* uniform_name, const T& value);

    RID get_custom_texture(const char* uniform_name) const;

    RID get_shader() const;

    bool has_custom_shader() const;

    const HashMap<String, UniformType>& get_uniforms() const;

    const HashMap<String, RID>& get_custom_textures() const;

    Color get_color() const;

    RID  get_texture() const;

private:

    Color color = Color::WHITE;
    RID texture  = INVALID_RID;

    RID shader = INVALID_RID;
    HashMap<String, UniformType> uniforms;
    HashMap<String, RID> custom_textures;
};


template<typename T>
CanvasMaterial& CanvasMaterial::set_shader_param(const char* uniform_name, const T& value) {
    uniforms[uniform_name] = value;
    return *this;
}