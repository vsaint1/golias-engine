#include "core/component/camera.h"


glm::mat4 Camera2D::get_view_matrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    view           = glm::translate(view, glm::vec3(-transform.position.x, -transform.position.y, 0.0f));
    view           = glm::rotate(view, glm::radians(transform.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    view           = glm::scale(view, glm::vec3(zoom));
    return view;
}
bool Camera2D::is_visible(const glm::vec3& position){
    glm::vec4 viewport = get_viewport();
    return position.x > viewport.x && position.x < viewport.x + viewport.z && position.y > viewport.y
        && position.y < viewport.y + viewport.w;
}

void Camera2D::resize(int view_width, int view_height) {
    _width  = view_width;
    _height = view_height;
}

glm::mat4 Camera2D::get_projection_matrix() const {
    glm::mat4 projection = glm::ortho(0.0f, (float) _width, (float) _height, 0.0f, -1.0f, 1.0f);

    return projection;
}

glm::vec4 Camera2D::get_viewport() const{


    float halfWidth  = _width / (2.0f * zoom);
    float halfHeight = _height / (2.0f * zoom);

    const auto position = transform.position;

    return {position.x - halfWidth, position.y - halfHeight, _width / zoom,
                     _height / zoom};
}
