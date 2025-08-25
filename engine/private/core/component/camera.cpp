#include "core/component/camera.h"

Camera2D::Camera2D() = default;


void Camera2D::set_offset(const glm::vec2& offset) { _offset = offset; }
glm::vec2 Camera2D::get_offset() const { return _offset; }

void Camera2D::set_zoom(const glm::vec2& zoom) { _zoom = zoom; }
 glm::vec2 Camera2D::get_zoom() const { return _zoom; }

void Camera2D::set_viewport(int width, int height) { _viewport_size = glm::vec2(width, height); }

glm::vec2 Camera2D::get_viewport() const { return _viewport_size; }

void Camera2D::follow(Node2D* target) { _follow_target = target; }

void Camera2D::process(double dt) {
    Node2D::process(dt);

    if (!_follow_target || _viewport_size.x == 0 || _viewport_size.y == 0) {
        return;
    }

    const auto pos = _follow_target->get_transform().position;

    _view_matrix = glm::translate(glm::mat4(1.f), glm::vec3(-pos + _offset + _viewport_size / 2.0f, 0.f));

    _view_matrix = glm::scale(_view_matrix, glm::vec3(_zoom, 1.f));
}

glm::mat4 Camera2D::get_view_matrix() const {
    return _view_matrix;
}
