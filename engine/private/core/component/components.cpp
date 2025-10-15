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

    LOG_DEBUG("Releasing model: %s", path.c_str());
    // automatically all ~destructors
    meshes.clear();
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
