#include "core/component/camera.h"


glm::mat4 Camera2D::GetViewMatrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    view           = glm::translate(view, glm::vec3(-transform.position.x, -transform.position.y, 0.0f));
    view           = glm::rotate(view, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    view           = glm::scale(view, glm::vec3(zoom));
    return view;
}
bool Camera2D::IsVisible(const glm::vec3& position){
    glm::vec4 viewport = GetViewport();
    return position.x > viewport.x && position.x < viewport.x + viewport.z && position.y > viewport.y
        && position.y < viewport.y + viewport.w;
}

void Camera2D::Resize(int view_width, int view_height) {
    width  = view_width;
    height = view_height;
}

glm::mat4 Camera2D::GetProjectionMatrix() const {
    glm::mat4 projection = glm::ortho(0.0f, (float) width, (float) height, 0.0f, -1.0f, 1.0f);

    return projection;
}

glm::vec4 Camera2D::GetViewport() const{


    float halfWidth  = width / (2.0f * zoom);
    float halfHeight = height / (2.0f * zoom);

    return glm::vec4(transform.position.x - halfWidth, transform.position.y - halfHeight, width / zoom,
                     height / zoom);
}
