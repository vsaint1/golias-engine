#include "servers/rendering/shader_material.h"


CanvasMaterial& CanvasMaterial::set_shader(const RID shader_rid) {
    shader = shader_rid;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_albedo(const Color& color) {
    albedo = color;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_texture(const RID tex) {
    texture = tex;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_float(const String& name, float value) {
    uniforms[name] = value;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_vec2(const String& name, const glm::vec2& value) {
    uniforms[name] = value;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_vec3(const String& name, const glm::vec3& value) {
    uniforms[name] = value;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_vec4(const String& name, const glm::vec4& value) {
    uniforms[name] = value;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_color(const String& name, const Color& color) {
    uniforms[name] = color.to_vec4();
    return *this;
}

CanvasMaterial& CanvasMaterial::set_int(const String& name, int value) {
    uniforms[name] = value;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_custom_texture(const String& name, RID tex) {
    custom_textures[name] = tex;
    return *this;
}

RID CanvasMaterial::get_custom_texture(const String& name) const {
    if (custom_textures.contains(name)) {
        return custom_textures.at(name);
    }

    return INVALID_RID;
}

Color CanvasMaterial::get_albedo() const {
    return albedo;
}
RID CanvasMaterial::get_texture() const {

    return texture;
}

RID CanvasMaterial::get_shader() const {
    return shader;
}

bool CanvasMaterial::has_custom_shader() const {
    return shader != INVALID_RID;
}

const HashMap<String, UniformType>& CanvasMaterial::get_uniforms() const {
    return uniforms;
}

const HashMap<String, RID>& CanvasMaterial::get_custom_textures() const {
    return custom_textures;
}
