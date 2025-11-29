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

    CanvasMaterial& set_albedo(const Color& color);

    CanvasMaterial& set_texture(RID tex);

    CanvasMaterial& set_float(const String& name, float value);

    CanvasMaterial& set_vec2(const String& name, const glm::vec2& value);

    CanvasMaterial& set_vec3(const String& name, const glm::vec3& value);

    CanvasMaterial& set_vec4(const String& name, const glm::vec4& value);

    CanvasMaterial& set_color(const String& name, const Color& color);

    CanvasMaterial& set_int(const String& name, int value);

    CanvasMaterial& set_custom_texture(const String& name, RID tex);

    RID get_custom_texture(const String& name) const;

    RID get_shader() const;

    bool has_custom_shader() const;

    const HashMap<String, UniformType>& get_uniforms() const;

    const HashMap<String, RID>& get_custom_textures() const;

    Color get_albedo() const;

    RID  get_texture() const;

private:

    Color albedo = Color::WHITE;
    RID texture  = INVALID_RID;

    RID shader = INVALID_RID;
    HashMap<String, UniformType> uniforms;
    HashMap<String, RID> custom_textures;
};
