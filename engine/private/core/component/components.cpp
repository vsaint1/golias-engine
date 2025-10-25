#include "core/component/components.h"

glm::mat4 Transform3D::get_matrix() const {
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
    mat           = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0));
    mat           = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0));
    mat           = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1));
    mat           = glm::scale(mat, scale);
    return mat;
}

void Camera3D::update_vectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front       = glm::normalize(direction);
}

glm::mat4 Camera3D::get_view_matrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera3D::get_projection_matrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
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
