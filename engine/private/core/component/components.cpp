#include "core/component/components.h"

#include "core/renderer/base_struct.h"

glm::mat4 Transform3D::get_matrix() const {
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
    mat           = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0));
    mat           = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0));
    mat           = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1));
    mat           = glm::scale(mat, scale);
    return mat;
}


void Camera3D::update_vectors() {
    glm::vec3 f;
    f.x   = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y   = sin(glm::radians(pitch));
    f.z   = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);

    right = glm::normalize(glm::cross(front, world_up));
    up    = glm::normalize(glm::cross(right, front));
}


void Material::bind(Shader* shader) const {
    shader->set_value("material.albedo", albedo);
    // shader->set_value("material.specular", specular);
    shader->set_value("material.metallic", metallic);
    shader->set_value("material.roughness", roughness);
    shader->set_value("material.ao", ao);
    shader->set_value("material.emissive", emissive);
    shader->set_value("material.emissiveStrength", emissive_strength);
    // shader->set_value("material.ior", ior);

    // Texture usage flags
    shader->set_value("USE_ALBEDO_MAP", use_albedo_map);
    // shader->set_value("USE_SPECULAR_MAP", use_specular_map);
    shader->set_value("USE_METALLIC_MAP", use_metallic_map);
    shader->set_value("USE_ROUGHNESS_MAP", use_roughness_map);
    shader->set_value("USE_NORMAL_MAP", use_normal_map);
    shader->set_value("USE_AO_MAP", use_ao_map);
    shader->set_value("USE_EMISSIVE_MAP", use_emissive_map);

    // Texture bindings
    if (use_albedo_map && albedo_map) {
        glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, albedo_map);
        shader->set_value("ALBEDO_MAP", ALBEDO_TEXTURE_UNIT);
    }

    if (use_specular_map && specular_map) {
        glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, specular_map);
        shader->set_value("SPECULAR_MAP", SPECULAR_TEXTURE_UNIT);
    }

    if (use_metallic_map && metallic_map) {
        glActiveTexture(GL_TEXTURE0 + METALLIC_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, metallic_map);
        shader->set_value("METALLIC_MAP", METALLIC_TEXTURE_UNIT);
    }

    if (use_roughness_map && roughness_map) {
        glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, roughness_map);
        shader->set_value("ROUGHNESS_MAP", ROUGHNESS_TEXTURE_UNIT);
    }

    if (use_normal_map && normal_map) {
        glActiveTexture(GL_TEXTURE0 + NORMAL_MAP_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, normal_map);
        shader->set_value("NORMAL_MAP", NORMAL_MAP_TEXTURE_UNIT);
    }

    if (use_ao_map && ao_map) {
        glActiveTexture(GL_TEXTURE0 + AMBIENT_OCCLUSION_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, ao_map);
        shader->set_value("AO_MAP", AMBIENT_OCCLUSION_TEXTURE_UNIT);
    }

    if (use_emissive_map && emissive_map) {
        glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, emissive_map);
        shader->set_value("EMISSIVE_MAP", EMISSIVE_TEXTURE_UNIT);
    }



}


glm::mat4 Camera3D::get_view(const Transform3D& transform) const {
    return glm::lookAt(transform.position, transform.position + front, up);
}

glm::mat4 Camera3D::get_projection(int w, int h) const {
    
    if (w <= 0 || h <= 0) {
        return glm::mat4(1.0f);
    }
    
    float aspect = (float)w / (float)h;
    
    if (aspect <= 0.0f || std::isinf(aspect) || std::isnan(aspect)) {
        return glm::mat4(1.0f);
    }
    
    return glm::perspective(glm::radians(fov), aspect, 0.1f, view_distance);
}

void Camera3D::move_forward(Transform3D& transform,float dt) {
    transform.position += front * speed * dt;
}

void Camera3D::move_backward(Transform3D& transform,float dt) {
    transform.position -= front * speed * dt;
}

void Camera3D::move_left(Transform3D& transform,float dt) {
    transform.position -= right * speed * dt;
}

void Camera3D::move_right(Transform3D& transform,float dt) {
    transform.position += right * speed * dt;
}

void Camera3D::look_at(float xoffset, float yoffset, float sensitivity) {
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    pitch = std::clamp(pitch, -89.0f, 89.0f);
    update_vectors();
}

void Camera3D::zoom(float yoffset) {
    fov = std::clamp(fov - yoffset, 1.0f, 90.0f);
}


glm::mat4 DirectionalLight3D::get_light_space_matrix(glm::mat4 camera_view, glm::mat4 camera_proj) const {
    glm::vec3 light_dir = glm::normalize(direction);

    glm::mat4 invCam = glm::inverse(camera_proj * camera_view);

    std::vector<glm::vec3> frustum_corners;
    for (int x = 0; x < 2; ++x)
        for (int y = 0; y < 2; ++y)
            for (int z = 0; z < 2; ++z) {
                glm::vec4 corner = invCam * glm::vec4(
                                       2.0f * x - 1.0f,
                                       2.0f * y - 1.0f,
                                       2.0f * z - 1.0f,
                                       1.0f
                                       );
                corner /= corner.w;
                frustum_corners.push_back(glm::vec3(corner));
            }

    glm::vec3 scene_center(0.0f);
    for (auto& c : frustum_corners)
        scene_center += c;
    scene_center /= static_cast<float>(frustum_corners.size());

    glm::vec3 light_position = scene_center - light_dir * 500.0f;

    glm::mat4 lightView = glm::lookAt(light_position, scene_center, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 min_bounds(FLT_MAX);
    glm::vec3 max_bounds(-FLT_MAX);
    for (auto& corner : frustum_corners) {
        auto ls    = glm::vec3(lightView * glm::vec4(corner, 1.0f));
        min_bounds = glm::min(min_bounds, ls);
        max_bounds = glm::max(max_bounds, ls);
    }

    constexpr float extend = 200.f;
    min_bounds.z -= extend;
    max_bounds.z += extend;

    glm::mat4 orthoProj = glm::ortho(
        min_bounds.x, max_bounds.x,
        min_bounds.y, max_bounds.y,
        -max_bounds.z, -min_bounds.z
        );

    return orthoProj * lightView;
}

glm::mat4 DirectionalLight3D::get_light_space_matrix() const {
    glm::mat4 lightProjection = glm::ortho(-shadowDistance, shadowDistance,
                                           -shadowDistance, shadowDistance,
                                           shadowNear, shadowFar);
    glm::vec3 lightPos  = -direction * (shadowDistance * 0.5f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

glm::mat4 Transform2D::get_matrix() const {
    glm::mat4 matrix(1.0f);
    matrix = glm::translate(matrix, glm::vec3(-origin, 0.0f));

    matrix = glm::scale(matrix, glm::vec3(scale, 1.0f));
    matrix = glm::rotate(matrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    matrix = glm::translate(matrix, glm::vec3(position + origin, 0.0f));

    return matrix;
}

glm::mat4 Camera2D::get_view_matrix() const {
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(-position, 0.0f));
    view = glm::rotate(view, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    return view;
}

glm::mat4 Camera2D::get_projection_matrix() const {

    if (viewport_size.x <= 0.0f || viewport_size.y <= 0.0f) {
        return glm::mat4(1.0f);
    }

    return glm::ortho(0.0f, viewport_size.x, viewport_size.y, 0.0f);
}

void Camera2D::move(const glm::vec2& offset) {
    position += offset;
}

void Camera2D::rotate(float radians) {
    rotation += radians;
}

void Camera2D::zoom_by(float factor) {
    zoom *= factor;
}

void Camera2D::set_zoom(float new_zoom) {
    zoom = new_zoom;
}

void Camera2D::look_at(const glm::vec2& target) {
    position = target;
}
