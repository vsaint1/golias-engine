#include "servers/rendering/shader_material.h"


CanvasMaterial& CanvasMaterial::set_shader(const RID shader_rid) {
    shader = shader_rid;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_color(const Color& col) {
    color = col;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_texture(const RID tex) {
    texture = tex;
    return *this;
}

CanvasMaterial& CanvasMaterial::set_custom_texture(const char* name, RID tex) {
    custom_textures[name] = tex;
    return *this;
}

RID CanvasMaterial::get_custom_texture(const char* uniform_name) const {
    if (custom_textures.contains(uniform_name)) {
        return custom_textures.at(uniform_name);
    }

    return INVALID_RID;
}

Color CanvasMaterial::get_color() const {
    return color;
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
