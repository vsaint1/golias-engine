#include "core/component/camera.h"


glm::mat4 Camera2D::GetViewMatrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    view           = glm::translate(view, glm::vec3(-transform.position.x, -transform.position.y, 0.0f));
    view           = glm::rotate(view, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    view           = glm::scale(view, glm::vec3(transform.scale.x, transform.scale.y, 1.0f));
    return view;
}
