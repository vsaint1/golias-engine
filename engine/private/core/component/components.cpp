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
    shader->set_value("material.metallic", metallic);
    shader->set_value("material.roughness", roughness);
    shader->set_value("material.ao", ao);
    shader->set_value("material.emissive", emissive);
    shader->set_value("material.emissiveStrength", emissiveStrength);

    // Texture usage flags
    shader->set_value("USE_ALBEDO_MAP", useAlbedoMap);
    shader->set_value("USE_METALLIC_MAP", useMetallicMap);
    shader->set_value("USE_ROUGHNESS_MAP", useRoughnessMap);
    shader->set_value("USE_NORMAL_MAP", useNormalMap);
    shader->set_value("USE_AO_MAP", useAOMap);
    shader->set_value("USE_EMISSIVE_MAP", useEmissiveMap);

    // Texture bindings
    if (useAlbedoMap && albedoMap) {
        glActiveTexture(GL_TEXTURE0 + ALBEDO_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, albedoMap);
        shader->set_value("ALBEDO_MAP", ALBEDO_TEXTURE_UNIT);
    }

    if (useMetallicMap && metallicMap) {
        glActiveTexture(GL_TEXTURE0 + METALLIC_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, metallicMap);
        shader->set_value("METALLIC_MAP", METALLIC_TEXTURE_UNIT);
    }

    if (useRoughnessMap && roughnessMap) {
        glActiveTexture(GL_TEXTURE0 + ROUGHNESS_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);
        shader->set_value("ROUGHNESS_MAP", ROUGHNESS_TEXTURE_UNIT);
    }

    if (useNormalMap && normalMap) {
        glActiveTexture(GL_TEXTURE0 + NORMAL_MAP_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        shader->set_value("NORMAL_MAP", NORMAL_MAP_TEXTURE_UNIT);
    }

    if (useAOMap && aoMap) {
        glActiveTexture(GL_TEXTURE0 + AMBIENT_OCCLUSION_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, aoMap);
        shader->set_value("AO_MAP", AMBIENT_OCCLUSION_TEXTURE_UNIT);
    }

    if (useEmissiveMap && emissiveMap) {
        glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, emissiveMap);
        shader->set_value("EMISSIVE_MAP", EMISSIVE_TEXTURE_UNIT);
    }



}


glm::mat4 Camera3D::get_view(const Transform3D& transform) const {
    return glm::lookAt(transform.position, transform.position + front, up);
}

glm::mat4 Camera3D::get_projection(int w, int h) const {
    return glm::perspective(glm::radians(fov), (float) w / (float) h, 0.1f, view_distance);
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


glm::mat4 DirectionalLight::get_light_space_matrix(glm::mat4 camera_view, glm::mat4 camera_proj) const {
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

glm::mat4 DirectionalLight::get_light_space_matrix() const {
    glm::mat4 lightProjection = glm::ortho(-shadowDistance, shadowDistance,
                                           -shadowDistance, shadowDistance,
                                           shadowNear, shadowFar);
    glm::vec3 lightPos  = -direction * (shadowDistance * 0.5f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}
