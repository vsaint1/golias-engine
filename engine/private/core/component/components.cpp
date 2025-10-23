#include "core/component/components.h"


Script::~Script() {
    if (lua_state) {

        lua_getglobal(lua_state, "_exit");
        if (lua_isfunction(lua_state, -1)) {
            if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK) {
                const char* err = lua_tostring(lua_state, -1);
                LOG_ERROR("Error running `_exit` in script %s: %s", path.c_str(), err);
                lua_pop(lua_state, 1);
            }
        }

        lua_close(lua_state);
        lua_state = nullptr;
    }
}

Model::~Model() {
    if (is_loaded && scene) {
        LOG_DEBUG("Releasing model: %s", path.c_str());
        if (importer) {
            importer->FreeScene();
        }
        scene = nullptr;
    }
    
    // automatically all ~destructors
    meshes.clear();
}

// NOTE: this is a very bad implemantation, needs to add CSM and better optimizations
glm::mat4 DirectionalLight::get_projection(const glm::mat4& camera_view, const glm::mat4& camera_proj) const {
    glm::vec3 light_dir = glm::normalize(direction);

    glm::mat4 invCam = glm::inverse(camera_proj * camera_view);

    std::vector<glm::vec3> frustum_corners;
    for (int x = 0; x < 2; ++x)
        for (int y = 0; y < 2; ++y)
            for (int z = 0; z < 2; ++z) {
                glm::vec4 corner = invCam * glm::vec4(
                                       2.0f * x - 1.0f, // -1 or +1
                                       2.0f * y - 1.0f, // -1 or +1
                                       2.0f * z - 1.0f, // -1 or +1
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


void Camera3D::update_vectors() {
    glm::vec3 f;
    f.x   = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y   = sin(glm::radians(pitch));
    f.z   = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);

    right = glm::normalize(glm::cross(front, world_up));
    up    = glm::normalize(glm::cross(right, front));
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
